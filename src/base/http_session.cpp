#include "http_session.hpp"

#include <boost/beast/core/stream_traits.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/websocket/rfc6455.hpp>
#include "websocket_session.hpp"

#include "handle_request.hpp"
#include "../http_request.hpp"

template<typename Req>
struct work_impl : work {
	ssl_http_session& self_;
	Req msg_;

	work_impl(ssl_http_session& self,
		Req&& msg) :
		self_(self),
		msg_(std::move(msg))
	{ }

	void operator()()
	{
		boost::beast::http::async_write(
			self_.stream(), msg_,
			boost::beast::bind_front_handler(&ssl_http_session::on_write,
				self_.shared_from_this(),
				msg_.need_eof()));
	}
};

response_queue::response_queue(ssl_http_session& self) :
	self_(self)
{
	static_assert(limit > 0, "queue limit must be positive");
	items_.reserve(limit);
}

// Returns true if we have reached the queue limit
bool response_queue::is_full() const
{ 
	return items_.size() >= limit; 
}

// Called when a message finishes sending
// Returns `true` if the caller should initiate a read
bool response_queue::on_write()
{
	BOOST_ASSERT(!items_.empty());
	auto const was_full = is_full();
	items_.erase(items_.begin());
	if (!items_.empty())
		(*items_.front())();
	return was_full;
}

// Called by the HTTP handler to send a response.
void response_queue::process(beast_response&& msg)
{
	add(std::make_unique<work_impl<beast_response>>(self_, std::move(msg)));
}
void response_queue::process_file(beast_response_file&& msg)
{
	add(std::make_unique<work_impl<beast_response_file>>(self_, std::move(msg)));
}

void response_queue::add(std::unique_ptr<work>&& item)
{
	// Allocate and store the work
	items_.emplace_back(std::move(item));

	// If there was no previous work, start this one
	if (items_.size() == 1)
		(*items_.front())();
}

ssl_http_session::ssl_http_session(boost::beast::tcp_stream&& stream, boost::asio::ssl::context& ctx, boost::beast::flat_buffer&& buffer) :
	queue_(*this),
	buffer_(std::move(buffer)),
	stream_(std::move(stream), ctx)
{ }

// Start the session
void ssl_http_session::run()
{
	// Set the timeout.
	boost::beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(5));

	// Perform the SSL handshake
	// Note, this is the buffered version of the handshake.
	stream_.async_handshake(
		boost::asio::ssl::stream_base::server, buffer_.data(),
		boost::beast::bind_front_handler(&ssl_http_session::on_handshake,
			shared_from_this()));
}

void ssl_http_session::do_eof()
{
	// Set the timeout.
	boost::beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(5));

	// Perform the SSL shutdown
	stream_.async_shutdown(boost::beast::bind_front_handler(&ssl_http_session::on_shutdown, shared_from_this()));
}

void ssl_http_session::do_read()
{
	// Construct a new parser for each message
	parser_.emplace();

	// Apply a reasonable limit to the allowed size
	// of the body in bytes to prevent abuse.
	parser_->body_limit(10000);

	// Set the timeout.
	boost::beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(60));

	// Read a request using the parser-oriented interface
	boost::beast::http::async_read(stream_, buffer_, *parser_, boost::beast::bind_front_handler(&ssl_http_session::on_read, shared_from_this()));
}

void ssl_http_session::on_read(boost::beast::error_code ec, std::size_t bytes_transferred)
{
	boost::ignore_unused(bytes_transferred);

	// This means they closed the connection
	if (ec == boost::beast::http::error::end_of_stream)
		return do_eof();

	if (ec)
		return theLog->error("[read]: ec: {} : {}", ec.value(), ec.message());

	// See if it is a WebSocket Upgrade
	if (boost::beast::websocket::is_upgrade(parser_->get()))
	{
		theLog->info("Upgrading to WebSocket");

		// Disable the timeout.
		// The websocket::stream uses its own timeout settings.
		boost::beast::get_lowest_layer(stream_).expires_after(std::chrono::minutes{30});

		// Create a websocket session, transferring ownership
		// of both the socket and the HTTP request.
		return make_websocket_session(release_stream(), parser_->release());
	}

	// Send the response
	handle_request(parser_->release(), queue_);

	// If we aren't at the queue limit, try to pipeline another request
	if (!queue_.is_full())
		do_read();
}

void ssl_http_session::on_write(bool close, boost::beast::error_code ec, std::size_t bytes_transferred)
{
	boost::ignore_unused(bytes_transferred);

	if (ec)
		return theLog->error("[write]: ec: {} : {}", ec.value(), ec.message());

	if (close)
	{
		// This means we should close the connection, usually because
		// the response indicated the "Connection: close" semantic.
		return do_eof();
	}

	// Inform the queue that a write completed
	if (queue_.on_write())
	{
		// Read another request
		do_read();
	}
}

void ssl_http_session::on_handshake(boost::beast::error_code ec, std::size_t bytes_used)
{
	if (ec)
		return theLog->error("[handshake]: code {} : {}", ec.value(), ec.message());

	// Consume the portion of the buffer used by the handshake
	buffer_.consume(bytes_used);

	do_read();
}

void ssl_http_session::on_shutdown(boost::beast::error_code ec)
{
	if (ec)
		return theLog->error("[shutdown]: code {} : {}", ec.value(), ec.message());

	// At this point the connection is closed gracefully
}
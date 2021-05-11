#include "http_session.hpp"

#include <boost/beast/core/stream_traits.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/websocket/rfc6455.hpp>
#include "websocket_session.hpp"

#include "handle_request.hpp"

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

const std::string& ssl_http_session::get_ssl_sid()
{
	if (ssl_id.empty())
	{
		SSL_SESSION* session = SSL_get1_session(stream_.native_handle());
		unsigned int ssl_id_length;
		const unsigned char* ssl_id_ptr = SSL_SESSION_get_id(session, &ssl_id_length);
		ssl_id = { reinterpret_cast<const char*>(ssl_id_ptr), ssl_id_length };
	}
	return ssl_id;
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
		return theLog->error("[read]: {}", ec.message());

	// See if it is a WebSocket Upgrade
	if (boost::beast::websocket::is_upgrade(parser_->get()))
	{
		theLog->info("Upgrading to WebSocket");

		// Disable the timeout.
		// The websocket::stream uses its own timeout settings.
		boost::beast::get_lowest_layer(stream_).expires_never();

		// Create a websocket session, transferring ownership
		// of both the socket and the HTTP request.
		return make_websocket_session(release_stream(), parser_->release());
	}

	// Send the response
	handle_request(get_ssl_sid(), parser_->release(), queue_);

	// If we aren't at the queue limit, try to pipeline another request
	if (!queue_.is_full())
		do_read();
}

void ssl_http_session::on_write(bool close, boost::beast::error_code ec, std::size_t bytes_transferred)
{
	boost::ignore_unused(bytes_transferred);

	if (ec)
		return theLog->error("[write]: {}", ec.message());

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
		return theLog->error("[handshake]: {}", ec.message());

	// Consume the portion of the buffer used by the handshake
	buffer_.consume(bytes_used);

	do_read();
}

void ssl_http_session::on_shutdown(boost::beast::error_code ec)
{
	if (ec)
		return theLog->error("[shutdown]: {}", ec.message());

	// At this point the connection is closed gracefully
}
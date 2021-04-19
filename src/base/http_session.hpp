#pragma once

#include <memory>
#include <vector>

#include <boost/optional/optional.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/basic_stream.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/core/stream_traits.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/websocket/rfc6455.hpp>

#include <Logging>
#include "handle_request.hpp"
#include "websocket_session.hpp"

template <class Derived>
class http_session 
{
	Derived& derived() { return static_cast<Derived&>(*this); }

	// This queue is used for HTTP pipelining.
	class queue 
	{
		// Maximum number of responses we will queue
		enum {
			limit = 8
		};

		// The type-erased, saved work item
		struct work {
			virtual ~work() = default;
			virtual void operator()() = 0;
		};

		http_session<Derived>& self_;
		std::vector<std::unique_ptr<work>> items_;

	public:
		explicit queue(http_session<Derived>& self) : 
			self_(self)
		{
			static_assert(limit > 0, "queue limit must be positive");
			items_.reserve(limit);
		}

		// Returns true if we have reached the queue limit
		bool is_full() const { return items_.size() >= limit; }

		// Called when a message finishes sending
		// Returns `true` if the caller should initiate a read
		bool on_write()
		{
			BOOST_ASSERT(!items_.empty());
			auto const was_full = is_full();
			items_.erase(items_.begin());
			if (!items_.empty())
				(*items_.front())();
			return was_full;
		}

		// Called by the HTTP handler to send a response.
		template <bool isRequest, class Body, class Fields>
		void operator()(boost::beast::http::message<isRequest, Body, Fields>&& msg)
		{
			// This holds a work item
			struct work_impl : work {
				http_session<Derived>& self_;
				boost::beast::http::message<isRequest, Body, Fields> msg_;

				work_impl(http_session& self,
					boost::beast::http::message<isRequest, Body, Fields>&& msg) : 
					self_(self),
					msg_(std::move(msg))
				{
				}

				void operator()()
				{
					boost::beast::http::async_write(
						self_.derived().stream(), msg_,
						boost::beast::bind_front_handler(&http_session::on_write,
							self_.derived().shared_from_this(),
							msg_.need_eof()));
				}
			};

			// Allocate and store the work
			items_.push_back(std::make_unique<work_impl>(self_, std::move(msg)));

			// If there was no previous work, start this one
			if (items_.size() == 1)
				(*items_.front())();
		}
	};

	std::shared_ptr<std::string const> doc_root_;
	queue queue_;

	// The parser is stored in an optional container so we can
	// construct it from scratch it at the beginning of each new message.
	boost::optional<boost::beast::http::request_parser<boost::beast::http::string_body>> parser_;

protected:
	boost::beast::flat_buffer buffer_;

public:
	// Construct the session
	http_session(boost::beast::flat_buffer buffer, std::shared_ptr<std::string const> const& doc_root) :
		doc_root_(doc_root),
		queue_(*this),
		buffer_(std::move(buffer))
	{ }

	void do_read()
	{
		// Construct a new parser for each message
		parser_.emplace();

		// Apply a reasonable limit to the allowed size
		// of the body in bytes to prevent abuse.
		parser_->body_limit(10000);

		// Set the timeout.
		boost::beast::get_lowest_layer(derived().stream())
			.expires_after(std::chrono::seconds(30));

		// Read a request using the parser-oriented interface
		boost::beast::http::async_read(derived().stream(), buffer_, *parser_, boost::beast::bind_front_handler(&http_session::on_read, derived().shared_from_this()));
	}

	void on_read(boost::beast::error_code ec, std::size_t bytes_transferred)
	{
		boost::ignore_unused(bytes_transferred);

		// This means they closed the connection
		if (ec == boost::beast::http::error::end_of_stream)
			return derived().do_eof();

		if (ec)
			return theLog->error("[read]: {}", ec.message());

		// See if it is a WebSocket Upgrade
		if (boost::beast::websocket::is_upgrade(parser_->get())) {
			// Disable the timeout.
			// The websocket::stream uses its own timeout settings.
			boost::beast::get_lowest_layer(derived().stream()).expires_never();

			// Create a websocket session, transferring ownership
			// of both the socket and the HTTP request.
			return make_websocket_session(derived().release_stream(), parser_->release());
		}

		// Send the response
		handle_request(*doc_root_, parser_->release(), queue_);

		// If we aren't at the queue limit, try to pipeline another request
		if (!queue_.is_full())
			do_read();
	}

	void on_write(bool close, boost::beast::error_code ec, std::size_t bytes_transferred)
	{
		boost::ignore_unused(bytes_transferred);

		if (ec)
			return theLog->error("[write]: {}", ec.message());

		if (close) {
			// This means we should close the connection, usually because
			// the response indicated the "Connection: close" semantic.
			return derived().do_eof();
		}

		// Inform the queue that a write completed
		if (queue_.on_write()) {
			// Read another request
			do_read();
		}
	}
};

//------------------------------------------------------------------------------

// Handles a plain HTTP connection
class plain_http_session : 
	public http_session<plain_http_session>,
	public std::enable_shared_from_this<plain_http_session> 
{
	boost::beast::tcp_stream stream_;

public:
	// Create the session
	plain_http_session(boost::beast::tcp_stream&& stream, boost::beast::flat_buffer&& buffer, std::shared_ptr<std::string const> const& doc_root) : 
		http_session<plain_http_session>(std::move(buffer), doc_root),
		stream_(std::move(stream))
	{}

	// Start the session
	void run() { this->do_read(); }

	// Called by the base class
	boost::beast::tcp_stream& stream() { return stream_; }

	// Called by the base class
	boost::beast::tcp_stream release_stream() { return std::move(stream_); }

	// Called by the base class
	void do_eof()
	{
		// Send a TCP shutdown
		boost::beast::error_code ec;
		stream_.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);

		// At this point the connection is closed gracefully
	}
};

//------------------------------------------------------------------------------

// Handles an SSL HTTP connection
class ssl_http_session : 
	public http_session<ssl_http_session>,
	public std::enable_shared_from_this<ssl_http_session> 
{
	boost::beast::ssl_stream<boost::beast::tcp_stream> stream_;

public:
	// Create the http_session
	ssl_http_session(boost::beast::tcp_stream&& stream, boost::asio::ssl::context& ctx, boost::beast::flat_buffer&& buffer, std::shared_ptr<std::string const> const& doc_root) :
		http_session<ssl_http_session>(std::move(buffer), doc_root), 
		stream_(std::move(stream), ctx)
	{
	}

	// Start the session
	void run()
	{
		// Set the timeout.
		boost::beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

		// Perform the SSL handshake
		// Note, this is the buffered version of the handshake.
		stream_.async_handshake(
			boost::asio::ssl::stream_base::server, buffer_.data(),
			boost::beast::bind_front_handler(&ssl_http_session::on_handshake,
				shared_from_this()));
	}

	// Called by the base class
	boost::beast::ssl_stream<boost::beast::tcp_stream>& stream() { return stream_; }

	// Called by the base class
	boost::beast::ssl_stream<boost::beast::tcp_stream> release_stream()
	{
		return std::move(stream_);
	}

	// Called by the base class
	void do_eof()
	{
		// Set the timeout.
		boost::beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

		// Perform the SSL shutdown
		stream_.async_shutdown(boost::beast::bind_front_handler(
			&ssl_http_session::on_shutdown, shared_from_this()));
	}

private:
	void on_handshake(boost::beast::error_code ec, std::size_t bytes_used)
	{
		if (ec)
			return theLog->error("[handshake]: {}", ec.message());

		// Consume the portion of the buffer used by the handshake
		buffer_.consume(bytes_used);

		do_read();
	}

	void on_shutdown(boost::beast::error_code ec)
	{
		if (ec)
			return theLog->error("[shutdown]: {}", ec.message());

		// At this point the connection is closed gracefully
	}
};
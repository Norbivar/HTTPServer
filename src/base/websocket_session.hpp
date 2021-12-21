#pragma once

#include <boost/beast/core/basic_stream.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <boost/beast/websocket/stream.hpp>

#include <Logging>

// Echoes back all received WebSocket messages.
// This uses the Curiously Recurring Template Pattern so that
// the same code works with both SSL streams and regular sockets.
template <class Derived>
class websocket_session 
{
	// Access the derived class, this is part of
	// the Curiously Recurring Template Pattern idiom.
	Derived& derived() { return static_cast<Derived&>(*this); }

	boost::beast::flat_buffer buffer_;

	// Start the asynchronous operation
	template <class Body, class Allocator>
	void do_accept(boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> req)
	{
		// Set suggested timeout settings for the websocket
		derived().ws().set_option(
			boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::server));

		// Set a decorator to change the Server of the handshake
		derived().ws().set_option(
			boost::beast::websocket::stream_base::decorator([](boost::beast::websocket::response_type& res) {
				res.set(boost::beast::http::field::server, std::string(BOOST_BEAST_VERSION_STRING) + " advanced-server-flex");
		}));

		// Accept the websocket handshake
		derived().ws().async_accept(
			req, boost::beast::bind_front_handler(&websocket_session::on_accept, derived().shared_from_this()));
	}

	void on_accept(boost::beast::error_code ec)
	{
		if (ec)
			return theLog->error("[accept]: {}", ec.message());

		// Read a message
		do_read();
	}

	void do_read()
	{
		// Read a message into our buffer
		derived().ws().async_read(
			buffer_, boost::beast::bind_front_handler(&websocket_session::on_read, derived().shared_from_this()));
	}

	void on_read(boost::beast::error_code ec, std::size_t bytes_transferred)
	{
		boost::ignore_unused(bytes_transferred);

		// This indicates that the websocket_session was closed
		if (ec == boost::beast::websocket::error::closed)
			return;

		if (ec)
			theLog->error("[read]: {}", ec.message());

		// Echo the message
		derived().ws().text(derived().ws().got_text());
		derived().ws().async_write(
			buffer_.data(),
			boost::beast::bind_front_handler(&websocket_session::on_write,
				derived().shared_from_this()));
	}

	void on_write(boost::beast::error_code ec, std::size_t bytes_transferred)
	{
		boost::ignore_unused(bytes_transferred);

		if (ec)
			return theLog->error("[write]: {}", ec.message());

		// Clear the buffer
		buffer_.consume(buffer_.size());

		// Do another read
		do_read();
	}

public:
	websocket_session() { theLog->info("VN: created websock"); }
	~websocket_session() { theLog->info("VN: removed websock"); }

	// Start the asynchronous operation
	template <class Body, class Allocator>
	void run(boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> req)
	{
		// Accept the WebSocket upgrade request
		do_accept(std::move(req));
	}
};

//------------------------------------------------------------------------------

// Handles a plain WebSocket connection
class plain_websocket_session : 
	public websocket_session<plain_websocket_session>,
	public std::enable_shared_from_this<plain_websocket_session> 
{
	boost::beast::websocket::stream<boost::beast::tcp_stream> ws_;

public:
	// Create the session
	explicit plain_websocket_session(boost::beast::tcp_stream&& stream) : 
		ws_(std::move(stream))
	{
	}

	// Called by the base class
	boost::beast::websocket::stream<boost::beast::tcp_stream>& ws() { return ws_; }
};

//------------------------------------------------------------------------------

// Handles an SSL WebSocket connection
class ssl_websocket_session : 
	public websocket_session<ssl_websocket_session>,
	public std::enable_shared_from_this<ssl_websocket_session> 
{
	boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream>> ws_;

public:
	// Create the ssl_websocket_session
	explicit ssl_websocket_session(boost::beast::ssl_stream<boost::beast::tcp_stream>&& stream) :
		ws_(std::move(stream))
	{
	}

	// Called by the base class
	boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream>>& ws() { return ws_; }
};

//------------------------------------------------------------------------------

template <class Body, class Allocator>
void make_websocket_session(boost::beast::tcp_stream stream, boost::beast::http::request<Body,boost::beast:: http::basic_fields<Allocator>> req)
{
	std::make_shared<plain_websocket_session>(std::move(stream))->run(std::move(req));
}

template <class Body, class Allocator>
void make_websocket_session(boost::beast::ssl_stream<boost::beast::tcp_stream> stream, boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> req)
{
	std::make_shared<ssl_websocket_session>(std::move(stream))->run(std::move(req));
}

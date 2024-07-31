#pragma once

#include <boost/beast/core/basic_stream.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <boost/beast/websocket/stream.hpp>

class ssl_websocket_session : public std::enable_shared_from_this<ssl_websocket_session>
{
public:
	//ssl_websocket_session();
	explicit ssl_websocket_session(boost::beast::ssl_stream<boost::beast::tcp_stream>&& stream);

	~ssl_websocket_session();

	template <class Body, class Allocator>
	void run(boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> req)
	{
		do_accept(std::move(req));
	}

	void send(const boost::beast::flat_buffer& msg);

	boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream>>& ws() { return websocket; }

private:
	boost::beast::flat_buffer receive_buffer{ 1 * 1024 * 3 };

	// Start the asynchronous operation
	template <class Body, class Allocator>
	void do_accept(boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> req)
	{
		// Set suggested timeout settings for the websocket
		websocket.set_option(
			boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::server));

		// Set a decorator to change the Server of the handshake
		websocket.set_option(
			boost::beast::websocket::stream_base::decorator([](boost::beast::websocket::response_type& res) {
				res.set(boost::beast::http::field::server, std::string(BOOST_BEAST_VERSION_STRING) + " advanced-server-flex");
				}));

		// Accept the websocket handshake
		websocket.async_accept(
			req, boost::beast::bind_front_handler(&ssl_websocket_session::on_accept, shared_from_this()));
	}

	void on_accept(boost::beast::error_code ec);

	void do_read();

	void on_read(boost::beast::error_code ec, std::size_t bytes_transferred);

	void on_write(boost::beast::error_code ec, std::size_t bytes_transferred);

	boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream>> websocket;
};

//------------------------------------------------------------------------------

template <class Body, class Allocator>
void make_websocket_session(boost::beast::ssl_stream<boost::beast::tcp_stream> stream, boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> req)
{
	std::make_shared<ssl_websocket_session>(std::move(stream))->run(std::move(req));
}

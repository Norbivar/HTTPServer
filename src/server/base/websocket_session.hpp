#pragma once

#include <boost/beast/core/basic_stream.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>
#include <boost/beast/websocket/stream.hpp>

#include <base/http_request.hpp>

class ssl_websocket_session : public std::enable_shared_from_this<ssl_websocket_session>
{
public:
	explicit ssl_websocket_session(boost::beast::ssl_stream<boost::beast::tcp_stream>&& stream);

	~ssl_websocket_session();

	void do_read();

	void send(const std::string& msg);
	void send(const boost::beast::flat_buffer& msg);
	void send(const boost::asio::const_buffer& msg);

	boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream>>& ws() { return websocket; }

	void do_accept(const beast_request& req);
	void do_refuse();

private:
	boost::beast::flat_buffer receive_buffer{ 1 * 1024 * 3 };

	std::string name{ "Unnamed" }; // maybe an enum?
	std::chrono::system_clock::time_point ws_creation_time{ std::chrono::system_clock::now() };
	bool verified{ false };

	void on_accept(boost::beast::error_code ec);
	void on_close(boost::beast::error_code ec);

	void on_read(boost::beast::error_code ec, std::size_t bytes_transferred);

	void on_write(boost::beast::error_code ec, std::size_t bytes_transferred);

	boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream>> websocket;
};

//------------------------------------------------------------------------------

//template <class Body, class Allocator>
//void make_websocket_session(boost::beast::ssl_stream<boost::beast::tcp_stream> stream, boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>> req)
//{
//	std::make_shared<ssl_websocket_session>(std::move(stream))->run(std::move(req));
//}

void make_websocket_session(boost::beast::ssl_stream<boost::beast::tcp_stream> stream, beast_request req);


//std::shared_ptr<ssl_websocket_session> construct_websocket_session(boost::beast::ssl_stream<boost::beast::tcp_stream> stream, beast_request req)
//{
//	return std::make_shared<ssl_websocket_session>(std::move(stream));
//}
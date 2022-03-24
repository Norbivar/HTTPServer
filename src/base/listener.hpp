#pragma once
#include <memory>

#include <boost/logic/tribool.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/basic_stream.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener>
{
public:
	listener(boost::asio::io_context& ioc, boost::asio::ssl::context& ctx, boost::asio::ip::tcp::endpoint endpoint);

	// Start accepting incoming connections
	void run() { do_accept(); }

private:
	void do_accept();
	void on_accept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket);

	boost::asio::io_context& ioc_;
	boost::asio::ssl::context& ctx_;
	boost::asio::ip::tcp::acceptor acceptor_;
};

// Detects SSL handshakes
class detect_session : public std::enable_shared_from_this<detect_session>
{
public:
	explicit detect_session(boost::asio::ip::tcp::socket&& socket, boost::asio::ssl::context& ctx);

	// Launch the detector
	void run();
	void on_detect(boost::beast::error_code ec, boost::tribool result);

private:
	boost::beast::tcp_stream stream_;
	boost::asio::ssl::context& ctx_;
	boost::beast::flat_buffer buffer_;
};
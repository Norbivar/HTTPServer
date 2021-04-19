#include "listener.hpp"

#include <boost/asio/strand.hpp>
#include <boost/beast/core/detect_ssl.hpp>

#include <Logging>
#include "http_session.hpp"

listener::listener(boost::asio::io_context& ioc, boost::asio::ssl::context& ctx, boost::asio::ip::tcp::endpoint endpoint, const std::shared_ptr<const std::string>& doc_root) : 
	ioc_(ioc), 
	ctx_(ctx), 
	acceptor_(boost::asio::make_strand(ioc)), 
	doc_root_(doc_root)
{
	boost::beast::error_code ec;

	// Open the acceptor
	acceptor_.open(endpoint.protocol(), ec);
	if (ec) 
	{
		theLog->error("[open]: {}", ec.message());
		return;
	}

	// Allow address reuse
	acceptor_.set_option(boost::asio::socket_base::reuse_address(true), ec);
	if (ec) 
	{
		theLog->error("[set_option]: {}", ec.message());
		return;
	}

	// Bind to the server address
	acceptor_.bind(endpoint, ec);
	if (ec) 
	{
		theLog->error("[bind]: {}", ec.message());
		return;
	}

	// Start listening for connections
	acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec);
	if (ec)
	{
		theLog->error("[listen]: {}", ec.message());
		return;
	}
}

void listener::do_accept()
{
	// The new connection gets its own strand
	acceptor_.async_accept(
		boost::asio::make_strand(ioc_),
		boost::beast::bind_front_handler(&listener::on_accept, shared_from_this()));
}

void listener::on_accept(boost::beast::error_code ec, boost::asio::ip::tcp::socket socket)
{
	if (ec)
	{
		theLog->error("[accept]: {}", ec.message());
	}
	else
	{
		// Create the detector http_session and run it
		std::make_shared<detect_session>(std::move(socket), ctx_, doc_root_)->run();
	}

	// Accept another connection
	do_accept();
}

detect_session::detect_session(boost::asio::ip::tcp::socket&& socket, boost::asio::ssl::context& ctx, std::shared_ptr<std::string const> const& doc_root) :
	stream_(std::move(socket)),
	ctx_(ctx),
	doc_root_(doc_root)
{
}

void detect_session::run()
{
	// Set the timeout.
	stream_.expires_after(std::chrono::hours(1));

	boost::beast::async_detect_ssl(
		stream_, buffer_,
		boost::beast::bind_front_handler(&detect_session::on_detect,
			this->shared_from_this()));
}

void detect_session::on_detect(boost::beast::error_code ec, boost::tribool result)
{
	if (ec)
		return theLog->error("[detect]: {}", ec.message());

	if (result)
	{
		// Launch SSL session
		std::make_shared<ssl_http_session>(std::move(stream_), ctx_,
			std::move(buffer_), doc_root_)->run();
		return;
	}

	// Launch plain session
	std::make_shared<plain_http_session>(std::move(stream_), std::move(buffer_),
		doc_root_)->run();
}
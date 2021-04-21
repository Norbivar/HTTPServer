#pragma once

#include <memory>
#include <thread>
#include <vector>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>

#include "base/handle_request.hpp"
#include "base/websocket_session.hpp"
#include "base/http_session.hpp"

//#include "router.hpp"

class WebServer
{
public:
	WebServer(const boost::asio::ip::address& address, const std::uint16_t port, const std::shared_ptr<std::string>& doc_root, const std::uint8_t numthreads);

	void Booststrap();
	int Run();
private:
	boost::asio::ip::address m_address;
	std::uint16_t m_port;
	std::shared_ptr<std::string> m_doc_root;
	std::uint8_t m_desired_thread_number;

	std::vector<std::thread> m_threads;
	boost::asio::io_context m_ioc;
	boost::asio::ssl::context m_ctx;
	//RoutingTable m_routing_table;
};
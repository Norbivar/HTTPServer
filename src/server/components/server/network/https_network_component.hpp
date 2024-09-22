#pragma once

#include "network_component.hpp"

#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ip/tcp.hpp>

class https_network_component : public network_component
{
public:
	https_network_component(const boost::asio::ip::address& address, std::uint16_t port, std::uint8_t threads_to_create);
	~https_network_component() = default;

	void setup_run() override;
    void await_finish() override;
	bool load_certificate() override;
    void stop() override;

private:
	std::uint8_t desired_thread_number;

	boost::asio::ip::address address;
	std::uint16_t port;
	boost::asio::io_context ioc;
	boost::asio::ssl::context ctx;
};
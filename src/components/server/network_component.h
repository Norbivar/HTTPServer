#pragma once

#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>

// Interfaces

class network_component
{
public:
	virtual void setup_run() = 0;
	virtual void await_finish() = 0;
	virtual bool load_certificate() = 0;
};


/////////////////////////////////////////////////////////////////////////////////
// Implementations

class server_network_component : public network_component
{
public:
	server_network_component(const boost::asio::ip::address & address, std::uint16_t port, std::uint8_t threads_to_create);
	void setup_run() override;
    void await_finish() override;
	bool load_certificate() override;

private:
	std::uint8_t desired_thread_number;

	boost::asio::ip::address address;
	std::uint16_t port;
	std::vector<std::thread> threads;
	boost::asio::io_context ioc;
	boost::asio::ssl::context ctx;

	boost::asio::signal_set interrupt_signal;
};
#pragma once

#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>

// Interfaces

class network_component
{
public:
	virtual ~network_component() = default;
	virtual void setup_run() = 0;
	virtual void await_finish() = 0;
	virtual bool load_certificate() = 0;
	virtual void stop() = 0;

	std::vector<std::thread> threads;
	std::unique_ptr<boost::asio::signal_set> interrupt_signal;
};


/////////////////////////////////////////////////////////////////////////////////
// Implementations

class https_network_component : public network_component
{
public:
	https_network_component(const boost::asio::ip::address & address, std::uint16_t port, std::uint8_t threads_to_create);
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

class websocket_network_component : public network_component
{

};
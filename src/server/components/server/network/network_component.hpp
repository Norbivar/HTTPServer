#pragma once

#include <memory>
#include <vector>
#include <thread>

#include <boost/asio/signal_set.hpp>

class network_component
{
public:
	virtual ~network_component() = default;
	virtual void setup_run() = 0;
	virtual void await_finish() = 0;
	virtual bool load_certificate() = 0;
	virtual void stop() = 0;
	// work provider?

	std::vector<std::thread> threads;
	std::unique_ptr<boost::asio::signal_set> interrupt_signal;
};
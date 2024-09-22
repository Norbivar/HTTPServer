#pragma once

#include "network_component.hpp"

class websocket_tracker;

// TODO: CURRENTLY UNUSED, AS BOOST BEAST IO_CONTEXCT WILL HANDLE THE ASYNC CALLS TO THE WEBSOCKETS !
// Get rid of this eventually.

class websocket_network_component : public network_component
{
public:
	websocket_network_component(std::uint8_t threads_to_create, websocket_tracker& sessions);
	~websocket_network_component() = default;

	void setup_run() override;
	void await_finish() override;
	bool load_certificate() override;
	void stop() override;

private:
	std::uint8_t desired_thread_number;
	std::atomic_bool stop_threads{ false };
	std::optional<std::thread> work_provider;

	websocket_tracker& sessions;
};
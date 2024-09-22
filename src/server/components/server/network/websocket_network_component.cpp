#include "websocket_network_component.hpp"

#include <Logging>
#include <Config>

#include "base/websocket_session.hpp"
#include "websocket_tracker.hpp"

websocket_network_component::websocket_network_component(std::uint8_t threads_to_create, websocket_tracker& sessions) :
	desired_thread_number{ threads_to_create },
	sessions{ sessions }
{
	assert(desired_thread_number != 0);
	threads.reserve(desired_thread_number);
}

void websocket_network_component::setup_run()
{
	for (auto i = 0; i < desired_thread_number; ++i)
	{
		std::thread work_handler{
			[this]() {
				while (!stop_threads) {

				}
			}
		};

		threads.emplace_back(std::move(work_handler));
	}

	//const auto read_sockets = [this]() {
	//	while (!stop_threads) {
	//		sessions.for_each([](const websocket_session_key& key) {
	//			key.websocket->do_read();
	//		});
	//	}
	//};
	//work_provider = std::thread{ read_sockets };

	theLog->info("-> WebSocket handling is runnning with threads: {} DONE", threads.size());
}

bool websocket_network_component::load_certificate()
{
	// TODO
	return false;
}

void websocket_network_component::await_finish()
{
	for (auto& thread : threads) {
		thread.join();
	}
}

void websocket_network_component::stop()
{
	stop_threads = true;
	await_finish();
}
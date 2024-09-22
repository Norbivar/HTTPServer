#include "WebServer.hpp"

#include <Logging>

#include "routing_table.hpp"
#include "session_tracker.hpp"
#include "websocket_tracker.hpp"
#include "database/sql/sql_manager.hpp"
#include "database/sql/sql_provider.hpp"
#include "components/server/network/https_network_component.hpp"
#include "components/server/network/websocket_network_component.hpp"


webserver::webserver() : webserver(
	theConfig->files_root,
	boost::asio::ip::make_address(theConfig->bind_ip),
	theConfig->port,
	theConfig->https_handler_threads)
{ }

webserver::webserver(const std::string& files_root, 
	const boost::asio::ip::address& address, 
	const uint16_t port, 
	const std::uint8_t https_threads) 
	:
	server_status{ status::starting },
	my_sql_manager{ std::make_unique<sql_manager>("Default Database", default_sql_provider{}) },
	my_session_tracker{ std::make_unique<session_tracker>() },
	my_websocket_tracker{ std::make_unique<websocket_tracker>() },
	my_routing_table{ std::make_unique<routing_table>() },
	my_https_network_component{ std::make_unique<https_network_component>(address, port, https_threads) }
	//my_websocket_network_component{ std::make_unique<websocket_network_component>(websocket_threads, *my_websocket_tracker) }
{
	theLog->info("Preparing WebServer ...");
}

webserver::~webserver()
{ }

void webserver::bootstrap()
{
	theLog->info("Bootstrapping WebServer ...");
	const auto dbh = my_sql_manager->acquire_handle();

	my_session_tracker->load_from_db(dbh);

	if (my_https_network_component->load_certificate())
		theLog->info("-> Certificates loaded DONE");
	else
		throw std::runtime_error{ "Failed to load certificate" };

	my_routing_table->register_all();
	theLog->info("-> Path mapping set up DONE");
	my_routing_table->print_stats();
}

int webserver::run()
{
	theLog->info("Setting up listener ...");

	my_https_network_component->setup_run();
	//my_websocket_network_component->setup_run();
	server_status = status::running;

	my_https_network_component->await_finish();
	//my_websocket_network_component->await_finish();
	server_status = status::stopping;

	theLog->info("WebServer stopped.");

	return EXIT_SUCCESS;
}
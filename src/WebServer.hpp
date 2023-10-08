#pragma once

#include <memory>
#include <thread>
#include <vector>

#include <boost/beast/core/string_type.hpp>
#include <boost/asio/ip/address.hpp>

#define theServer webserver::instance()

class routing_table;
class session_tracker;
class sql_manager;
class network_component;

class webserver
{
public:
	static webserver& instance()
	{
		static webserver instance;
		return instance;
	}

	enum status
	{
		unknown,
		starting,
		running,
		stopping
	};

	webserver();
	webserver(const std::string& files_root, const std::string& doc_root, const boost::asio::ip::address& address, const std::uint16_t port, const std::uint8_t numthreads);
	~webserver();

	void bootstrap();
	int run();

	const boost::beast::string_view get_doc_root() const { return doc_root; }
	const auto& get_routing_table() const { return *my_routing_table; }

	auto& get_session_tracker() { return *my_session_tracker; }
	const auto& get_session_tracker() const { return *my_session_tracker; }

	auto& get_sql_manager() { return *my_sql_manager; }
	const auto& get_sql_manager() const { return *my_sql_manager; }

	status get_status() const { return server_status; }

	std::uint64_t fetch_add_request_count() { return request_count.fetch_add(1); }

private:
	status server_status{ status::unknown };
	std::atomic_uint64_t request_count{ 0 };

	const std::string doc_root;

	std::unique_ptr<network_component> my_network_component;
	std::unique_ptr<sql_manager> my_sql_manager;
	std::unique_ptr<routing_table> my_routing_table;
	std::unique_ptr<session_tracker> my_session_tracker;
};
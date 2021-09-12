#pragma once

#include <memory>
#include <thread>
#include <vector>

#include <boost/beast/core/string_type.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>

#define theServer webserver::instance()

class routing_table;
class session_tracker;
class sql_manager;

class webserver
{
public:
	static webserver& instance()
	{
		static webserver instance;
		return instance;
	}

	webserver();
	webserver(std::string&& doc_root, const boost::asio::ip::address& address, const std::uint16_t port, const std::uint8_t numthreads);
	~webserver();

	void bootstrap();
	int run();

	const boost::beast::string_view get_doc_root() const { return m_doc_root; }
	const routing_table& get_routing_table() const { return *m_routing_table; }
	session_tracker& get_session_tracker() { return *m_session_tracker; }
	sql_manager& get_sql_manager() { return *m_sql_manager; }

private:
	void load_server_certificate();

	boost::asio::ip::address m_address;
	std::uint16_t m_port;
	const std::string m_doc_root;
	std::uint8_t m_desired_thread_number;

	std::vector<std::thread> m_threads;
	boost::asio::io_context m_ioc;
	boost::asio::ssl::context m_ctx;

	std::unique_ptr<routing_table> m_routing_table;
	std::unique_ptr<session_tracker> m_session_tracker;
	std::unique_ptr<sql_manager> m_sql_manager;
};
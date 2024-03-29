#include "routing_table.hpp"

#include <chrono>
#include <thread>

#include <Logging>
#include <conditional_lock.hpp>

#include "base/http_request.hpp"
#include "base/http_response.hpp"
#include "session_tracker.hpp"
#include "auth/auth.hpp"
#include "WebServer.hpp"

void routing_table::register_all()
{
	using method = boost::beast::http::verb;

	register_path(method::post, "/login", authentication::request_login, {}, false);
	register_path(method::post, "/register", authentication::request_register, {}, false);

	register_path(method::post, "/test_session", authentication::test_session, {}, true);

}

void routing_table::register_path(boost::beast::http::verb verb,
	std::string&& path,
	const routing_handler& handler,
	const routing_access_predicate& pred,
	const bool need_session)
{
	auto _ = unique_conditional_lock(m_mutex, []() { return theServer.get_status() == webserver::status::running; });

	auto& verb_table_it = m_tables[verb];
	verb_table_it.emplace(std::move(path), routing_entry{ handler, pred, need_session });
}

routing_table::routing_lookup_result routing_table::lookup(boost::beast::http::verb verb, const std::string& path) const
{
	routing_lookup_result entry{ false, {} };

	auto _ = shared_conditional_lock(m_mutex, []() { return theServer.get_status() == webserver::status::running; });

	const auto verb_table_it = m_tables.find(verb);
	if (verb_table_it == m_tables.end())
		return entry;

	const auto it = verb_table_it->second.find(path);
	if (it != verb_table_it->second.end())
	{
		entry.second = it;
		entry.first = true;
	}
	return entry;
}

void routing_table::print_stats() const
{
	theLog->info("	Routing Table entries by methods:");
	for (const auto& [v, t] : m_tables)
		theLog->info("		{} : {} entries", to_string(v), t.size());
}
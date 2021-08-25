#include "routing_table.hpp"

#include <chrono>
#include <thread>

#include <Logging>

#include "http_request.hpp"
#include "http_response.hpp"
#include "session_tracker.hpp"
#include "auth/auth.hpp"

void routing_table::register_all()
{
	using method = boost::beast::http::verb;

	register_path(method::post, "/login", authentication::on_login, {}, false);

}

void routing_table::register_path(boost::beast::http::verb verb, 
	std::string&& path, 
	const routing_handler& handler, 
	const routing_access_predicate& pred,  
	const bool need_session)
{
	auto& verb_table_it = m_tables[verb];
	verb_table_it.emplace(std::move(path), routing_entry{ handler, pred, need_session });
}

std::pair<bool, routing_table::table_t::const_iterator> routing_table::lookup(boost::beast::http::verb verb, const std::string& path) const
{
	std::pair<bool, routing_table::table_t::const_iterator> entry{ false, nullptr };

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
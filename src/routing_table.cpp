#include "routing_table.hpp"

#include <Logging>

namespace Routing
{
	bool all_access_predicate() { return true; }
}

void routing_table::register_path(boost::beast::http::verb verb, std::string&& path, routing_handler&& handler, routing_access_predicate&& pred)
{
	auto& verb_table_it = m_tables[verb];
	verb_table_it.emplace(std::move(path), std::move(routing_entry{ std::move(handler), std::move(pred) }));
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

void routing_table::register_all()
{
	using method = boost::beast::http::verb;

	register_path(method::get, "/test", [](const boost::property_tree::ptree& req, boost::property_tree::ptree& resp) {
		theLog->error("Received that mofo GET test request!");
		for (const auto& arg : req)
			theLog->error("Arg: {} : {}", arg.first, arg.second.data());

		resp.add("norbi", "yes please");
	});

	register_path(method::post, "/test", [](const boost::property_tree::ptree& arguments, boost::property_tree::ptree& resp) {
		theLog->error("Received that mofo test request!");
		for (const auto& arg : arguments)
			theLog->error("Arg: {} : {}", arg.first, arg.second.data());
	});

	register_path(method::post, "/test_deny", [](const boost::property_tree::ptree& arguments, boost::property_tree::ptree& resp) { theLog->error("Received that mofo test deny request!"); }, []() { return false; });

}
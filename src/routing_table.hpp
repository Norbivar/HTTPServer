#pragma once

#include <iostream>

#include <boost/beast/http/verb.hpp>
#include <boost/function.hpp>
#include <boost/unordered_map.hpp>
#include <boost/property_tree/ptree.hpp>

namespace Routing
{
	bool all_access_predicate();
}

class routing_table
{
	 // TODO: add some sort of session parameter to these when I figure that out
	using routing_access_predicate = boost::function<bool(void)>;
	using routing_handler = boost::function<void(const boost::property_tree::ptree&, boost::property_tree::ptree&)>;

	struct routing_entry
	{
		routing_handler handler = nullptr;
		routing_access_predicate access_predicate = nullptr;
	};
	using table_t = boost::unordered_map<std::string, routing_entry>;

public:
	routing_table() = default;

	/** Registers a [HTTP Method, Path] pair with he given handler and an optional access predicate. Tables are created where needed. */
	void register_path(boost::beast::http::verb verb, std::string&& path, routing_handler&& handler, routing_access_predicate&& pred = Routing::all_access_predicate);

	/** Looks up athe HTTP Method, and if a corresponding table can be found, looks up the path in there. 
	* Returns a pair, where the boolean is true if the following iterator is valid. 
	*/
	std::pair<bool, table_t::const_iterator> lookup(boost::beast::http::verb verb, const std::string& path) const;

	/** Logs out the tables. */
	void print_stats() const;

	/** Registers all paths into the routing table. Used to provide a unified place for all requestable urls. */
	void register_all();

private:
	boost::unordered_map<boost::beast::http::verb, table_t> m_tables;
};


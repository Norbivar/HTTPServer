#pragma once

#include <boost/variant.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/function.hpp>
#include <boost/unordered_map.hpp>
#include <boost/property_tree/ptree.hpp>

#include "base/http_session.hpp"

class session_info;

class routing_table
{
	using routing_access_predicate = boost::variant<
		boost::blank,
		bool(*)(void),
		bool(*)(const session_info&)
	>;
	using routing_handler = boost::variant< 
		void(*)(const std::string&, const boost::property_tree::ptree&, boost::property_tree::ptree&), // ssl session id + input params/body + response body
		void(*)(const boost::property_tree::ptree&, boost::property_tree::ptree&), // input params/body + response body
		void(*)(session_info&, const boost::property_tree::ptree&, boost::property_tree::ptree&) // user session + input params/body + response body
	>;

	struct routing_entry
	{
		routing_handler handler;
		routing_access_predicate access_predicate;
	};
	using table_t = boost::unordered_map<std::string, routing_entry>;

public:
	routing_table() = default;

	/** Registers a [HTTP Method, Path] pair with he given handler and an optional access predicate. Tables are created where needed. */
	void register_path(boost::beast::http::verb verb, std::string&& path, const routing_handler& handler, const routing_access_predicate& pred = routing_access_predicate{});

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


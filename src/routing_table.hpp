#pragma once

#include <boost/variant.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/function.hpp>
#include <boost/unordered_map.hpp>

class http_request;
class http_response;

#include "base/http_session.hpp"

class session_info;

class routing_table
{
	using routing_access_predicate = bool(*)(const http_request&);
	using routing_handler = void(*)(const http_request&, http_response&);

	struct routing_entry
	{
		routing_handler handler;
		routing_access_predicate access_predicate;
		bool need_session;
	};
	using table_t = boost::unordered_map<std::string, routing_entry>;

public:
	using routing_lookup_result = std::pair<bool, table_t::const_iterator>;

	routing_table() = default;

	/** Registers a [HTTP Method, Path] pair with he given handler and an optional access predicate. Tables are created where needed. */
	void register_path(boost::beast::http::verb verb, std::string&& path, const routing_handler& handler, const routing_access_predicate& pred = routing_access_predicate{}, const bool need_session = true);

	/** Looks up athe HTTP Method, and if a corresponding table can be found, looks up the path in there. 
	* Returns a pair, where the boolean is true if the following iterator is valid. 
	*/
	routing_lookup_result lookup(boost::beast::http::verb verb, const std::string& path) const;

	/** Logs out the tables. */
	void print_stats() const;

	/** Registers all paths into the routing table. Used to provide a unified place for all requestable urls. */
	void register_all();

private:
	boost::unordered_map<boost::beast::http::verb, table_t> m_tables;
};


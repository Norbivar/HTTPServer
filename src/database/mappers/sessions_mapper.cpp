#include "sessions_mapper.hpp"

#include <boost/algorithm/string/join.hpp>
#include <Logging>
#include <pqxx/transaction>
#include <pqxx/result>
#include "../sql/sql_handle.hpp"

namespace
{
	std::vector<session_element> sessions_from_sql(const pqxx::result& res)
	{
		std::vector<session_element> elements;
		elements.reserve(res.size());
		for (const auto it : res)
		{
			session_element elem{ it[0].c_str(), it[1].as<std::uint32_t>() };
			elem.session_creation_time = std::chrono::system_clock::time_point{ std::chrono::seconds{it[2].as<std::uint64_t>()} };
			elem.last_request_time = std::chrono::system_clock::time_point{ std::chrono::seconds{it[3].as<std::uint64_t>()} };
			elements.emplace_back(std::move(elem));
		}
		return elements;
	}
}

std::string sessions_mapper::filter_t::to_string(const sql_handle& db) const
{
	std::vector<std::string> where;
	if (sid)
		where.emplace_back(fmt::format("session_id LIKE '{}'", db.escape(*sid)));
	if (account_id)
		where.emplace_back(fmt::format("account_id = {}", *account_id));

	if (where.empty())
		return "";
	else
		return "WHERE " + boost::join(where, " AND ");
}

std::vector<session_element> sessions_mapper::get_raw(const sql_handle& db, const filter_t& filter, const boost::optional<std::uint32_t>& limit)
{
	const auto limit_str = limit ? fmt::format("LIMIT {}", *limit) : "";

	auto work = db.start();
	const auto res = work->exec(fmt::format("SELECT session_id, account_id, EXTRACT(epoch FROM creation_time)::integer, EXTRACT(epoch FROM last_request_time)::integer FROM httpserver.sessions {} {};",
		filter.to_string(db),
		limit_str)
	);

	return sessions_from_sql(res);
}

boost::optional<session_element> sessions_mapper::get(const sql_handle& db, const filter_t& filter)
{
	auto get_res = get_raw(db, filter, 1);
	if (!get_res.empty())
		return std::move(get_res.front());
	else
		return boost::none;
}

std::vector<session_element> sessions_mapper::get_all(const sql_handle& db, const filter_t& filter)
{
	return get_raw(db, filter);
}

void sessions_mapper::insert(sql_handle& db, const std::vector<std::string>& sessions)
{
	if (sessions.empty())
		return;

	auto work = db.start();
	work->exec0(
		fmt::format("INSERT INTO httpserver.sessions VALUES {}; ",
			boost::join(sessions, ","))
	);
}
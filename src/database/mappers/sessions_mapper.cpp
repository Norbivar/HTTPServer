#include "sessions_mapper.hpp"

#include <Logging>
#include "../libs/format.hpp"
#include "../sql/sql_includes.hpp"

namespace
{
	session_element session_from_sql(const sql::ResultSet& res)
	{
		session_element elem{ res.getString(1), res.getUInt(2) };
		elem.session_creation_time = std::chrono::system_clock::time_point{ std::chrono::seconds{res.getUInt64(3)} };
		elem.last_request_time = std::chrono::system_clock::time_point{ std::chrono::seconds{res.getUInt64(4)} };
		return elem;
	}
}

std::string sessions_mapper::filter_t::to_string(sql_handle& db) const
{
	std::vector<std::string> where;
	if (sid)
		where.emplace_back(format_string("sessionid='%1%'", db.escape(sid->c_str())));
	if (account_id)
		where.emplace_back(format_string("accountid=%1%", *account_id));

	if (where.empty())
		return "";
	else
		return "WHERE " + boost::algorithm::join(where, " AND ");
}

std::vector<session_element> sessions_mapper::get_raw(sql_handle& db, const filter_t& filter, const boost::optional<std::uint32_t>& limit)
{
	const auto limit_str = limit ? format_string("LIMIT %1%", *limit) : "";

	std::unique_ptr<sql::PreparedStatement> pstmt{
		db->prepareStatement(
			format_string("SELECT sessionid, accountid, UNIX_TIMESTAMP(creationtime), UNIX_TIMESTAMP(lastrequesttime) FROM sessions %1% %2%;", 
				filter.to_string(db), 
				limit_str
			)
		)
	};

	std::unique_ptr<sql::ResultSet> res{ pstmt->executeQuery() };
	if (!res->rowsCount())
		return {};

	std::vector<session_element> result;
	result.reserve(res->rowsCount());
	while (res->next())
		result.emplace_back(session_from_sql(*res));

	return result;
}

boost::optional<session_element> sessions_mapper::get(sql_handle& db, const filter_t& filter)
{
	auto get_res = get_raw(db, filter, 1);
	if (!get_res.empty())
		return std::move(get_res.front());
	else
		return boost::none;
}

std::vector<session_element> sessions_mapper::get_all(sql_handle& db, const filter_t& filter)
{
	return get_raw(db, filter);
}

void sessions_mapper::insert(sql_handle& db, const std::vector<std::string>& sessions)
{
	if (sessions.empty())
		return;

	std::unique_ptr<sql::PreparedStatement> pstmt{
		db->prepareStatement(
			format_string("REPLACE INTO sessions VALUES %1%;",
				boost::join(sessions, ","))
		)
	};
	pstmt->executeUpdate();
}
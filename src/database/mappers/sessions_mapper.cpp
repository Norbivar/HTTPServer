#include "sessions_mapper.hpp"

#include "../libs/format.hpp"
#include "../sql/sql_includes.hpp"

namespace
{
	session_element session_from_sql(const sql::ResultSet& res)
	{
		session_element elem { res.getString(1), res.getUInt(2) };
		elem.session_creation_time = std::chrono::system_clock::time_point{ std::chrono::seconds{res.getUInt64(3)} };
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

boost::optional<session_element> sessions_mapper::get(sql_handle& db, const filter_t& filter)
{
	std::unique_ptr<sql::PreparedStatement> pstmt{
		db->prepareStatement(
			format_string("SELECT sessionid, accountid, UNIX_TIMESTAMP(creationtime) FROM sessions %1% LIMIT 1;", filter.to_string(db))
		)
	};

	std::unique_ptr<sql::ResultSet> res{ pstmt->executeQuery() };
	if (!res->rowsCount() || !res->next())
		return boost::none;

	return session_from_sql(*res);
}

std::vector<session_element> sessions_mapper::get_all(sql_handle& db, const filter_t& filter)
{
	std::unique_ptr<sql::PreparedStatement> pstmt{
		db->prepareStatement(
			format_string("SELECT sessionid, accountid, UNIX_TIMESTAMP(creationtime) FROM sessions %1%;", filter.to_string(db))
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

void sessions_mapper::insert(sql_handle& db, const object_range<session_element>& range)
{
	for (const auto& item : range)
	{
		std::unique_ptr<sql::PreparedStatement> pstmt{
			db->prepareStatement(
				format_string("INSERT INTO sessions VALUES('%1%', %2%, FROM_UNIXTIME(%3%));",
					db.escape(item.session_id).c_str(),
					item.account_id,
					std::chrono::duration_cast<std::chrono::seconds>(item.session_creation_time.time_since_epoch()).count())
			)
		};

		pstmt->executeUpdate();
	}
	db->commit();
}
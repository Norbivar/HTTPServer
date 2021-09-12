#include "accounts_mapper.hpp"

#include <memory>
#include <boost/algorithm/string/join.hpp>
#include <boost/format.hpp>
#include <Logging>

#include "../sql/sql_includes.hpp"

namespace
{
	account_element account_from_sql(const sql::ResultSet& res)
	{
		account_element elem;
		elem.id = res.getUInt(1);
		elem.username = res.getString(2);
		elem.email = res.getString(3);
		elem.creationtime = std::chrono::system_clock::time_point{std::chrono::seconds{res.getUInt64(4)}};
		return elem;
	}
}

std::string accounts_mapper::filter_t::to_string(sql_handle& db) const
{
	std::vector<std::string> where;
	if (username)
		where.emplace_back(boost::str(boost::format("username='%1%'") % db.escape(username->c_str())));
	if (password)
		where.emplace_back(boost::str(boost::format("password='%1%'") % db.escape(password->c_str())));
	if (email)
		where.emplace_back(boost::str(boost::format("email='%1%'") % db.escape(email->c_str())));
	
	if (where.empty())
		return "";
	else
		return "WHERE " + boost::algorithm::join(where, OR ? " OR " : " AND ");
}

std::vector<account_element> accounts_mapper::get(sql_handle& db, const filter_t& filter)
{
	std::unique_ptr<sql::PreparedStatement> pstmt{ 
		db->prepareStatement(
			boost::str(boost::format("SELECT id, username, email, UNIX_TIMESTAMP(creationtime) FROM accounts %1%;") % filter.to_string(db))
		)
	};

	std::unique_ptr<sql::ResultSet> res{ pstmt->executeQuery() };
	if (!res->rowsCount())
		return {};

	std::vector<account_element> result;
	result.reserve(res->rowsCount());
	while (res->next())
		result.emplace_back(account_from_sql(*res));
	
	return result;
}

bool accounts_mapper::insert(sql_handle& db, const account_element& element)
{
	std::unique_ptr<sql::PreparedStatement> pstmt{db->prepareStatement("INSERT INTO accounts(username, password, email, creationtime) VALUES(?, ?, ?, FROM_UNIXTIME(?));")};
	pstmt->setString(1, element.username);
	pstmt->setString(2, element.password);
	pstmt->setString(3, element.email);
	pstmt->setInt64(4, std::chrono::duration_cast<std::chrono::seconds>(element.creationtime.time_since_epoch()).count());

	return pstmt->executeUpdate() == 1;
}
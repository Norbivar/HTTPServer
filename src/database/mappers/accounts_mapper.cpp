#include "accounts_mapper.hpp"

#include <memory>
#include <Logging>

#include <boost/algorithm/string/join.hpp>
#include <pqxx/transaction>
#include <pqxx/result>
#include "../libs/spdlog/fmt/fmt.h"
#include "../sql/sql_handle.hpp"

namespace
{
	std::vector<account_element> accounts_from_sql(const pqxx::result& res)
	{
		std::vector<account_element> elements;
		elements.reserve(res.size());
		for (const auto it : res)
		{
			account_element elem;
			elem.id = it[0].as<std::uint32_t>();
			elem.username = it[1].c_str();
			elem.email = it[2].c_str();
			elem.creationtime = std::chrono::system_clock::time_point{ std::chrono::seconds{it[3].as<std::uint64_t>()} };
			elements.emplace_back(std::move(elem));
		}

		return elements;
	}
}

std::string accounts_mapper::filter_t::to_string(const sql_handle& db) const
{
	std::vector<std::string> where;
	if (username)
		where.emplace_back(fmt::format("username LIKE '{}'", db.escape(*username)));
	if (password)
		where.emplace_back(fmt::format("password LIKE '{}'", db.escape(*password)));
	if (email)
		where.emplace_back(fmt::format("email LIKE '{}'", db.escape(*email)));

	if (where.empty())
		return "";
	else
		return "WHERE " + boost::join(where, OR ? " OR " : " AND ");
}

std::vector<account_element> accounts_mapper::get(const sql_handle& db, const filter_t& filter)
{
	auto work = db.start();
	const auto res = work->exec(
		fmt::format("SELECT id, username, email, EXTRACT(epoch FROM creation_time)::integer FROM httpserver.accounts {};", filter.to_string(db))
	);

	return accounts_from_sql(res);
}

bool accounts_mapper::insert(sql_handle& db, const account_element& element)
{
	try
	{
		auto work = db.start();
		work->exec0(
			fmt::format("INSERT INTO httpserver.accounts('username', 'password', 'email', 'creation_time') VALUES('{}', '{}', '{}', TO_TIMESTAMP({}));",
				db.escape(element.username),
				db.escape(element.password),
				db.escape(element.email),
				std::chrono::duration_cast<std::chrono::seconds>(element.creationtime.time_since_epoch()).count())
		);
	}
	catch (const std::exception& ex)
	{
		theLog->error(ex);
		return false;
	}
	return true;
}
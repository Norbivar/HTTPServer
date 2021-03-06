#pragma once

#include <boost/optional/optional.hpp>

#include "../../id_types.hpp"
#include "../session_element.hpp"
#include "../sql/sql_helpers.hpp"

class sql_handle;

struct sessions_mapper
{
	struct filter_t
	{
		boost::optional<id::session> sid;
		boost::optional<id::account> account_id;

		std::string to_string(const sql_handle& db) const;
	};

	static boost::optional<session_element> get(const sql_handle& db, const filter_t& filter = {});
	static std::vector<session_element> get_all(const sql_handle& db, const filter_t& filter = {});
	static void insert(sql_handle& db, const insert_range_as_sql& sessions);

private:
	static std::vector<session_element> get_raw(const sql_handle& db, const filter_t& filter = {}, const boost::optional<std::uint32_t>& limit = boost::none);
};
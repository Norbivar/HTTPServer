#pragma once

#include <boost/optional/optional.hpp>
#include <boost/range/iterator_range.hpp>

#include <object_range.hpp>

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

		std::string to_string(sql_handle& db) const;
	};

	static boost::optional<session_element> get(sql_handle& db, const filter_t& filter = {});
	static std::vector<session_element> get_all(sql_handle& db, const filter_t& filter = {});
	static void insert(sql_handle& db, const std::vector<std::string>& sessions);
};
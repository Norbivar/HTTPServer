#pragma once

#include <boost/optional/optional.hpp>
#include "../account_element.hpp"

class sql_handle;

struct accounts_mapper
{
	struct filter_t
	{
		bool OR = false;
		boost::optional<std::string> username;
		boost::optional<std::string> password;
		boost::optional<std::string> email;

		std::string to_string(sql_handle& db) const;
	};

	static std::vector<account_element> get(sql_handle& db, const filter_t& filter);
	static bool insert(sql_handle& db, const account_element& element);
};
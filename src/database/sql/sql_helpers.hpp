#pragma once

#include <vector>

template<typename T>
std::vector<std::string> to_sql(const T& range)
{
	std::vector<std::string> ret;
	ret.reserve(std::distance(std::begin(range), std::end(range)));

	for (const auto& elem : range)
	{
		std::string str_format = std::remove_reference<decltype(elem)>::type::to_sql_string(elem);
		if (!str_format.empty())
			ret.emplace_back(std::move(str_format));
	}

	return ret;
}

template<typename T>
std::vector<std::string> to_sql(sql_handle& db, const T& range)
{
	std::vector<std::string> ret;
	ret.reserve(std::distance(std::begin(range), std::end(range)));

	for (const auto& elem : range)
	{
		std::string str_format = std::remove_reference<decltype(elem)>::type::to_sql_string(db, elem);
		if (!str_format.empty())
			ret.emplace_back(std::move(str_format));
	}

	return ret;
}
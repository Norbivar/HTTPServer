#pragma once

#include <boost/algorithm/string/join.hpp>
#include <boost/format.hpp>

template<typename... Args>
std::string format_string(const char* form, Args... args)
{
	const auto res = (boost::format(form) % ... % args);
	return boost::str(res);
}
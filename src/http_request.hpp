#pragma once

#include "../libs/nlohmann/json.hpp"
#include <boost/beast/http.hpp>

using beast_request = boost::beast::http::message<true, boost::beast::http::string_body, boost::beast::http::fields>;

class http_request
{
public:
	http_request(beast_request&& b);
	std::string sid;

	template<typename T>
	boost::optional<T> get_optional(const std::string& name) const
	{
		if (_base_request_data.count(name))
			return _base_request_data[name].get<T>();
		else return boost::none;
	}

	template<typename T>
	T get(const std::string& name) const
	{
		return _base_request_data[name].get<T>();
	}

	template<typename T>
	T operator[](const std::string& name) const
	{
		return get<T>(name);
	}

	auto& base() { return _base; }
private:
	beast_request _base;
	nlohmann::json _base_request_data;
};
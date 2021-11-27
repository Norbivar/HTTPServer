#pragma once

#include "../libs/nlohmann/json.hpp"
#include <boost/beast/http.hpp>
#include <optional>

using beast_request = boost::beast::http::message<true, boost::beast::http::string_body, boost::beast::http::fields>;

struct session_element;

class http_request
{
public:
	http_request(beast_request&& b, const std::string& addr);

	std::string sid;
	std::string address;
	std::shared_ptr<session_element> session; // set from outside

	template<typename T>
	T get(const std::string& name) const
	{
		if constexpr (std::is_same_v<T, std::optional<T::value_type>> || std::is_same_v<T, boost::optional<T::value_type>>)
		{
			if (_base_request_data.count(name))
				return _base_request_data[name].get<T::value_type>();
			else return boost::none;
		}
		else
		{
			return _base_request_data[name].get<T>();
		}
	}

	template<typename T>
	T get(const std::string& name, const T& defaultval) const
	{
		if (_base_request_data.count(name))
			return _base_request_data[name].get<T>();
		else
			return defaultval;
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
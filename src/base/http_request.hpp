#pragma once

#include "../libs/nlohmann/json.hpp"
#include <boost/beast/http.hpp>
#include <optional>

using beast_request = boost::beast::http::message<true, boost::beast::http::string_body, boost::beast::http::fields>;

namespace threadsafe
{
	template<typename T>
	class element;
}

struct session_element;

namespace http
{
	class json : public nlohmann::json
	{
		template <typename F = boost::optional<T>>
		T get() const noexcept()
		{
			
			return 
		}
	};
}

class http_request
{
public:
	http_request(std::uint64_t id, beast_request&& b, const std::string& addr);

	std::uint64_t id;
	std::string sid;
	std::string address;
	std::shared_ptr<threadsafe::element<session_element>> session; // set from outside

	auto& base() { return _base; }

	const auto& params() const { return _base_request_data; }
	const auto& operator[](const std::string& name) const { return _base_request_data[name]; }
	const auto& operator[](const char* name) const { return operator[](std::string{ name }); }
private:
	beast_request _base;
	http::json _base_request_data;
};
#pragma once

#include <optional>
#include <boost/beast/http.hpp>

#include "../libs/nlohmann/json.hpp"
#include "../libs/compile_traits.hpp"

using beast_request = boost::beast::http::message<true, boost::beast::http::string_body, boost::beast::http::fields>;

namespace threadsafe
{
	template<typename T>
	class element;
}

struct session_element;
class http_request;

namespace nlohmann // small, non-intrusive extension to the json lib
{
	template<typename T>
	T get(const nlohmann::json& json, const std::string& name);

	// Const char* forwardor
	template<typename T>
	T get(const nlohmann::json& json, const char* name);

	// Just to be able to read directly from req without calling .params() all the time
	template<typename T>
	T get(const http_request& req, const std::string& name);

	template<typename T>
	T get(const http_request& req, const char* name);
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

	template<typename T>
	T get(const std::string& name) const { return nlohmann::get<T>(_base_request_data, name); }

	template<typename T>
	T get(const char* name) const { return nlohmann::get<T>(_base_request_data, name); }
private:
	beast_request _base;
	nlohmann::json _base_request_data;
};

#include "../libs/nlohmann_additions.hpp"
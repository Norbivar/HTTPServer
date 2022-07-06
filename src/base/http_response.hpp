#pragma once

#include "../libs/nlohmann/json.hpp"
#include <boost/beast/http.hpp>

using beast_response = boost::beast::http::response<boost::beast::http::string_body>;
using beast_response_empty = boost::beast::http::response<boost::beast::http::empty_body>;
using beast_response_file = boost::beast::http::response<boost::beast::http::file_body>;

class http_response
{
public:
	http_response(std::uint32_t version, const bool keepalive);

	boost::beast::http::status response_code() const { return _base.result(); }
	void response_code(const boost::beast::http::status res) { _base.result(res); }
	
	// Creates a 'Set-Cookie' flag on the HTTP Response, so the browser will set the defined cookie
	void set_cookie(const std::string& cookie) { _base.set("Set-Cookie", cookie); }

	// Sets the response json to the specified.
	void set_response(const nlohmann::json& js) { _base_response_data = js; }
	void set_response(nlohmann::json&& js) { _base_response_data = std::move(js); }
	
	// Operator syntax for the set_response.
	void operator=(const nlohmann::json& js) { set_response(js); }
	void operator=(nlohmann::json&& js) { set_response(std::move(js)); }
	
	// Provides access to the result json objects highest level properties.
	auto& operator[](const std::string& name)
	{
		return _base_response_data[name];
	}

	beast_response&& prepare_release();
private:
	beast_response _base;
	nlohmann::json _base_response_data;
};
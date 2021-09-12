#pragma once

#include "../libs/nlohmann/json.hpp"
#include <boost/beast/http.hpp>

using beast_response = boost::beast::http::response<boost::beast::http::string_body>;
using beast_response_empty = boost::beast::http::response<boost::beast::http::empty_body>;
using beast_response_file = boost::beast::http::response<boost::beast::http::file_body>;

class http_response
{
public:
	http_response(const std::uint32_t version, const bool keepalive);

	boost::beast::http::status response_code() const { return _base.result(); }
	void response_code(const boost::beast::http::status res) { _base.result(res); }
	void push_back(const nlohmann::json& js) { _base_response_data.push_back(js); }
	void push_back(nlohmann::json&& js) { _base_response_data.push_back(std::move(js)); }
	void emplace_back(const nlohmann::json& js) { _base_response_data.emplace_back(js); }
	void emplace_back(nlohmann::json&& js) { _base_response_data.emplace_back(std::move(js)); }

	auto& operator[](const std::string& name)
	{
		return _base_response_data[name];
	}

	beast_response&& prepare_release();
private:
	beast_response _base;
	nlohmann::json _base_response_data;
};
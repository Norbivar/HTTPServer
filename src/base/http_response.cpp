#include "http_response.hpp"

#include <boost/beast/version.hpp>

http_response::http_response(const std::uint32_t version, const bool keepalive) 
{ 
	_base = { boost::beast::http::status::ok, version };
	_base.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
	_base.set(boost::beast::http::field::content_type, "application/json");
	_base.keep_alive(keepalive);
}

beast_response&& http_response::prepare_release()
{
	std::string response_str = _base_response_data.dump();
	_base.content_length(response_str.size());
	_base.body() = std::move(response_str);
	return std::move(_base);
}
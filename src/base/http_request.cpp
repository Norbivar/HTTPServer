#include "http_request.hpp"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <Logging>

#include "../database/session_element.hpp"

const std::string extract_cookie(const std::string& cookie, const std::string& label)
{
	auto cookie_start = cookie.find(label + "=");
	if (cookie_start != cookie.npos)
	{
		cookie_start += label.size() + 1;
		const auto cookie_end = cookie.find(";", cookie_start);
		const auto found = cookie.substr(cookie_start, cookie_end - cookie_start);
		if (found != "deleted") // againts client force persist cookie
			return found;
	}
	return {};
}

http_request::http_request(std::uint64_t id, beast_request&& b, const std::string& addr) :
	id{ id },
	_base{ std::move(b) },
	address{ addr }
{
	const auto cookies = _base.find(boost::beast::http::field::cookie);
	if (cookies != _base.end())
	{
		const auto cookie_str = cookies->value().to_string();

		sid = extract_cookie(cookie_str, "SID");
	}

	auto req_target_stripped = _base.target().to_string();
	if (_base.method() == boost::beast::http::verb::get)
	{
		const auto get_method_question_mark = req_target_stripped.find_first_of('?');
		if (get_method_question_mark != std::string::npos)
		{
			std::vector<std::string> get_params;
			boost::split(get_params, req_target_stripped.substr(get_method_question_mark + 1), boost::is_any_of("&"));

			req_target_stripped = req_target_stripped.substr(0, get_method_question_mark);
			for (const auto& param : get_params)
			{
				const auto equal_operator = param.find_first_of('=');
				if (equal_operator == std::string::npos)
					throw std::invalid_argument{ "ill formed get: no equal" };

				const auto lhs = param.substr(0, equal_operator);
				const auto rhs = param.substr(equal_operator + 1);
				if (lhs.empty() || rhs.empty())
					throw std::invalid_argument{ "ill formed get: invalid parameter (no name/no value)" };

				_base_request_data.push_back({ lhs, rhs });
			}
		}
	}
	else if (_base.method() == boost::beast::http::verb::post)
	{
		std::stringstream ss;
		ss << _base.body();
		_base_request_data = nlohmann::json::parse(ss);
	}
}
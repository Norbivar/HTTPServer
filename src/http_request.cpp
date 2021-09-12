#include "http_request.hpp"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <Logging>

#include "session_info.hpp"

http_request::http_request(beast_request&& b) :
	_base{std::move(b)}
{
	const auto sid_it = _base.find(boost::beast::http::field::authorization);
	if (sid_it != _base.end())
		sid = sid_it->value().to_string();

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
					throw std::invalid_argument{"ill formed get"};

				const auto lhs = param.substr(0, equal_operator);
				const auto rhs = param.substr(equal_operator + 1);
				if (lhs.empty() || rhs.empty())
					throw std::invalid_argument{"ill formed get"};

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
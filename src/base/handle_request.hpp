#pragma once

#include <boost/beast/version.hpp>
#include <boost/beast/core/string_type.hpp>
#include <boost/beast/http.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <Logging>
#include <json.hpp>

#include "../webserver.hpp"
#include "../session_tracker.hpp"

namespace
{
	template<typename session_pair>
	struct access_predicate_visitor : public boost::static_visitor<>
	{
		access_predicate_visitor(bool& out, const session_pair& in) : output{out}, sess_pair{in} {}

		void operator()(boost::blank) const { }
		void operator()(bool(handler)(void)) const { output = handler(); }
		void operator()(bool(handler)(const session_info&)) const
		{
			if (sess_pair.first && sess_pair.second->info)
				output = handler(*sess_pair.second->info);
			else
				theLog->error("Tried calling access predicate that requires session_info without one!");
		}
	private:
		const session_pair& sess_pair; 
		bool& output;
	};

	template<typename session_pair>
	struct routing_handler_visitor : public boost::static_visitor<>
	{
		routing_handler_visitor(const std::string& ssl_sid, session_pair& session, const nlohmann::json& in, nlohmann::json& out) : 
			ssl_sid{ssl_sid}, sess_pair{session}, in{in}, out{out} {}

		void operator()(void(handler)(const nlohmann::json&, nlohmann::json&)) const { handler(in, out); }
		void operator()(void(handler)(session_info&, const nlohmann::json&, nlohmann::json&)) const
		{
			if (sess_pair.first && sess_pair.second->info)
				handler(*sess_pair.second->info, in, out);
			else
				theLog->error("Tried calling access predicate that requires session_info without one!");
		}
		void operator()(void(handler)(const std::string& ssl_sid, const nlohmann::json&, nlohmann::json&)) const
		{
			handler(ssl_sid, in, out);
		}

	private:
		const std::string& ssl_sid;
		const nlohmann::json& in;
		nlohmann::json& out;
		session_pair& sess_pair;
	};
}

// Return a reasonable mime type based on the extension of a file.
boost::beast::string_view mime_type(boost::beast::string_view path);

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string path_cat(boost::beast::string_view base, boost::beast::string_view path);

template<typename Req>
boost::beast::http::response<boost::beast::http::string_body> set_bad_request(const Req& req, boost::beast::string_view why)
{
	boost::beast::http::response<boost::beast::http::string_body> res
	{
		boost::beast::http::status::bad_request,
		req.version()
	};
	res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(boost::beast::http::field::content_type, "text/html");
	res.keep_alive(req.keep_alive());
	res.body() = std::string(why);
	res.prepare_payload();
	return res;
}

template<typename Req>
boost::beast::http::response<boost::beast::http::string_body> set_not_found(const Req& req)
{
	boost::beast::http::response<boost::beast::http::string_body> res
	{
		boost::beast::http::status::not_found,
		req.version()
	};
	res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(boost::beast::http::field::content_type, "text/html");
	res.keep_alive(req.keep_alive());
	res.body() = "The resource '" + std::string(req.target()) + "' was not found.";
	res.prepare_payload();
	return res;
}

template<typename Req>
boost::beast::http::response<boost::beast::http::string_body> set_server_error(const Req& req, boost::beast::string_view what)
{
	boost::beast::http::response<boost::beast::http::string_body> res
	{
		boost::beast::http::status::internal_server_error,
		req.version()
	};
	res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(boost::beast::http::field::content_type, "text/html");
	res.keep_alive(req.keep_alive());
	res.body() = "An error occurred: '" + std::string(what) + "'";
	res.prepare_payload();
	return res;
}

template<typename Req>
boost::beast::http::response<boost::beast::http::string_body> set_unauthorized(const Req& req)
{
	boost::beast::http::response<boost::beast::http::string_body> res
	{
		boost::beast::http::status::unauthorized,
		req.version()
	};
	res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(boost::beast::http::field::content_type, "text/html");
	res.keep_alive(req.keep_alive());
	res.prepare_payload();
	return res;
}

template <class Body, class Allocator, class Send>
void handle_request(
	const std::string& ssl_id,
	boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>>&& req, 
	Send&& send)
{
	// Request path must be absolute and not contain "..".
	if (req.target().empty() || req.target()[0] != '/' || req.target().find("..") != boost::beast::string_view::npos)
		return send(set_bad_request(req, "Illegal request-target"));

	// Build the path to the requested file
	std::string path = path_cat(webserver::instance().get_doc_root(), req.target());
	if (req.target().back() == '/')
		path.append("index.html");

	const auto mime = mime_type(path);
	if (mime != "application/text")
	{
		// Attempt to open the file
		boost::beast::error_code ec;
		boost::beast::http::file_body::value_type body;
		body.open(path.c_str(), boost::beast::file_mode::scan, ec);

		// Handle the case where the file doesn't exist
		if (ec == boost::beast::errc::no_such_file_or_directory)
			return send(set_not_found(req));

		// Handle an unknown error
		if (ec)
			return send(set_server_error(req, ec.message()));

		// Cache the size since we need it after the move
		const auto size = body.size();

		// Respond to HEAD request
		if (req.method() == boost::beast::http::verb::head)
		{
			boost::beast::http::response<boost::beast::http::empty_body> res{ boost::beast::http::status::ok, req.version() };
			res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
			res.set(boost::beast::http::field::content_type, mime_type(path));
			res.content_length(size);
			res.keep_alive(req.keep_alive());
			return send(std::move(res));
		}

		// Respond to GET request
		boost::beast::http::response<boost::beast::http::file_body> res
		{
			std::piecewise_construct, 
			std::make_tuple(std::move(body)),
			std::make_tuple(boost::beast::http::status::ok, req.version())
		};
		res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(boost::beast::http::field::content_type, mime_type(path));
		res.content_length(size);
		res.keep_alive(req.keep_alive());
		return send(std::move(res));
	}
	else
	{
		auto req_target_stripped = req.target().to_string();
		nlohmann::json arguments; // POST body OR get url parameters

		if (req.method() == boost::beast::http::verb::get)
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
						return send(set_bad_request(req, "ill formed get"));

					const auto lhs = param.substr(0, equal_operator);
					const auto rhs = param.substr(equal_operator + 1);
					if (lhs.empty() || rhs.empty())
						return send(set_bad_request(req, "ill formed get"));

					arguments.push_back({lhs, rhs});
				}
			}
		}
		else if (req.method() == boost::beast::http::verb::post)
		{
			std::stringstream ss;
			ss << req.body();
			try
			{
				arguments = nlohmann::json::parse(ss);
			}
			catch (const std::exception& e)
			{
				theLog->error("Json parse error: {}", e.what());
				boost::beast::http::response<boost::beast::http::string_body> res{ boost::beast::http::status::bad_request, req.version() };
				res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
				return send(std::move(res));
			}
		}

		const auto [success, it] = webserver::instance().get_routing_table().lookup(req.method(), req_target_stripped);
		if (!success)
			return send(set_not_found(req));

		auto session_pair = webserver::instance().get_session_tracker().find_by_ssl(ssl_id);
		if (it->second.access_predicate.which() != 0)
		{
			bool result = false;
			access_predicate_visitor visitor {result, session_pair};
			it->second.access_predicate.apply_visitor(visitor);
			if (!result)
				return send(set_unauthorized(req));
		}

		nlohmann::json response;
		routing_handler_visitor visitor {ssl_id, session_pair, arguments, response};
		try
		{
			it->second.handler.apply_visitor(visitor);
		}
		catch (const std::exception& e)
		{
			theLog->error("Root catched exception: {}", e.what());
			boost::beast::http::response<boost::beast::http::string_body> res{ boost::beast::http::status::internal_server_error, req.version() };
			res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
			return send(std::move(res));
		}

		std::string response_str = response.dump();

		boost::beast::http::response<boost::beast::http::string_body> res{ boost::beast::http::status::ok, req.version() };
		res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(boost::beast::http::field::content_type, "application/json");
		res.content_length(response_str.size());
		res.keep_alive(req.keep_alive());
		res.body() = std::move(response_str);
		return send(std::move(res));
	}
}
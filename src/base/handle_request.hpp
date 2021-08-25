#pragma once

#include <boost/beast/core/string_type.hpp>
#include <boost/beast/version.hpp>

#include <Logging>

#include "../http_request.hpp"
#include "../http_response.hpp"
#include "../webserver.hpp"
#include "../session_tracker.hpp"

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

template<typename Req>
boost::beast::http::response<boost::beast::http::empty_body> respond_head_request(const std::string& path, const std::uint64_t size, Req& req)
{
	boost::beast::http::response<boost::beast::http::empty_body> res{ boost::beast::http::status::ok, req.version() };
	res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(boost::beast::http::field::content_type, mime_type(path));
	res.content_length(size);
	res.keep_alive(req.keep_alive());
	return res;
}

template <class Body, class Allocator, class Send>
void handle_request(boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>>&& req, Send&& send)
{
	// Request path must be absolute and not contain "..".
	if (req.target().empty() || req.target()[0] != '/' || req.target().find("..") != boost::beast::string_view::npos)
		return send(set_bad_request(req, "Illegal request-target"));

	// Build the path to the requested file
	std::string path = path_cat(theServer.get_doc_root(), req.target());
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
			return send(respond_head_request(path, size, req));

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
		auto get_method_question_mark = req_target_stripped.find_first_of('?');
		if (get_method_question_mark != std::string::npos)
			req_target_stripped = req_target_stripped.substr(0, get_method_question_mark);

		const auto [success, it] = theServer.get_routing_table().lookup(req.method(), req_target_stripped);
		if (!success)
			return send(set_not_found(req));

		try
		{
			http_request request{std::move(req)};
			auto session_pair = theServer.get_session_tracker().find_by_session_id(request.sid);
			if (it->second.need_session && !session_pair.first)
				return send(set_unauthorized(req));

			if (it->second.access_predicate)
			{
				bool result = it->second.access_predicate(request);
				if (!result)
					return send(set_unauthorized(req));
			}

			http_response response {request.base().version(), req.keep_alive()};
			it->second.handler(request, response);
			
			return send(std::move(response.prepare_release()));
		}
		catch (const nlohmann::json::parse_error& e)
		{
			theLog->error("Json parse error: {}", e.what());
			boost::beast::http::response<boost::beast::http::string_body> res{ boost::beast::http::status::bad_request, req.version() };
			res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
			return send(std::move(res));
		}
		catch (const std::invalid_argument& e)
		{
			theLog->error("Input parse error: {}", e.what());
			boost::beast::http::response<boost::beast::http::string_body> res{ boost::beast::http::status::bad_request, req.version() };
			res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
			return send(std::move(res));
		}
		catch (const std::exception& e)
		{
			theLog->error("Root catched exception: {}", e.what());
			boost::beast::http::response<boost::beast::http::string_body> res{ boost::beast::http::status::internal_server_error, req.version() };
			res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
			return send(std::move(res));
		}
	}
}
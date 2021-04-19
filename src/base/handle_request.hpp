#pragma once

#include <boost/beast/version.hpp>
#include <boost/beast/core/string_type.hpp>
#include <boost/beast/http.hpp>

#include <Logging>

// Return a reasonable mime type based on the extension of a file.
boost::beast::string_view mime_type(boost::beast::string_view path);

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string path_cat(boost::beast::string_view base, boost::beast::string_view path);

template <class Body, class Allocator, class Send>
void handle_request(boost::beast::string_view doc_root, boost::beast::http::request<Body, boost::beast::http::basic_fields<Allocator>>&& req, Send&& send)
{
	theLog->error("Request: {}", req.target().data());

	// Returns a bad request response
	const auto bad_req = [&req](boost::beast::string_view why)
	{
		boost::beast::http::response<boost::beast::http::string_body> res{
			boost::beast::http::status::bad_request,
			req.version() 
		};
		res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(boost::beast::http::field::content_type, "text/html");
		res.keep_alive(req.keep_alive());
		res.body() = std::string(why);
		res.prepare_payload();
		return res;
	};

	// Returns a not found response
	const auto not_found = [&req](boost::beast::string_view target)
	{
		boost::beast::http::response<boost::beast::http::string_body> res{
			boost::beast::http::status::not_found,
			req.version()
		};
		res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(boost::beast::http::field::content_type, "text/html");
		res.keep_alive(req.keep_alive());
		res.body() = "The resource '" + std::string(target) + "' was not found.";
		res.prepare_payload();
		return res;
	};

	// Returns a server error response
	auto const server_error = [&req](boost::beast::string_view what)
	{
		boost::beast::http::response<boost::beast::http::string_body> res{
			boost::beast::http::status::internal_server_error,
			req.version()
		};
		res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(boost::beast::http::field::content_type, "text/html");
		res.keep_alive(req.keep_alive());
		res.body() = "An error occurred: '" + std::string(what) + "'";
		res.prepare_payload();
		return res;
	};

	// Make sure we can handle the method
	if (req.method() != boost::beast::http::verb::get && req.method() != boost::beast::http::verb::head)
		return send(bad_req("Unknown HTTP-method"));

	// Request path must be absolute and not contain "..".
	if (req.target().empty() || req.target()[0] != '/' || req.target().find("..") != boost::beast::string_view::npos)
		return send(bad_req("Illegal request-target"));

	// Build the path to the requested file
	std::string path = path_cat(doc_root, req.target());
	if (req.target().back() == '/')
		path.append("index.html");

	// Attempt to open the file
	boost::beast::error_code ec;
	boost::beast::http::file_body::value_type body;
	body.open(path.c_str(), boost::beast::file_mode::scan, ec);

	// Handle the case where the file doesn't exist
	if (ec == boost::beast::errc::no_such_file_or_directory)
		return send(not_found(req.target()));

	// Handle an unknown error
	if (ec)
		return send(server_error(ec.message()));

	// Cache the size since we need it after the move
	auto const size = body.size();

	// Respond to HEAD request
	if (req.method() == boost::beast::http::verb::head) {
		boost::beast::http::response<boost::beast::http::empty_body> res{ boost::beast::http::status::ok, req.version() };
		res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
		res.set(boost::beast::http::field::content_type, mime_type(path));
		res.content_length(size);
		res.keep_alive(req.keep_alive());
		return send(std::move(res));
	}

	// Respond to GET request
	boost::beast::http::response<boost::beast::http::file_body> res{
		std::piecewise_construct, std::make_tuple(std::move(body)),
		std::make_tuple(boost::beast::http::status::ok, req.version())
	};
	res.set(boost::beast::http::field::server, BOOST_BEAST_VERSION_STRING);
	res.set(boost::beast::http::field::content_type, mime_type(path));
	res.content_length(size);
	res.keep_alive(req.keep_alive());
	return send(std::move(res));
}
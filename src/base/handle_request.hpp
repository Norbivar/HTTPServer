#pragma once

#include <boost/beast/core/string_type.hpp>

#include "../http_request.hpp"
#include "../http_response.hpp"

class response_queue;

// Return a reasonable mime type based on the extension of a file.
boost::beast::string_view mime_type(boost::beast::string_view path);

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string path_cat(boost::beast::string_view base, boost::beast::string_view path);

beast_response set_bad_request(const beast_request& req, boost::beast::string_view why);
beast_response set_not_found(const beast_request& req);
beast_response set_server_error(const beast_request& req, boost::beast::string_view what);
beast_response set_unauthorized(const beast_request& req);
beast_response respond_head_request(const std::string& path, const std::uint64_t size, const beast_request& req);

void handle_request(beast_request&& req, response_queue& resp_queue);
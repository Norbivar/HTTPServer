#pragma once

#include <boost/beast/core/string_type.hpp>

#include "http_request.hpp"
#include "http_response.hpp"

class response_queue;

// Return a reasonable mime type based on the extension of a file.
boost::beast::string_view mime_type(boost::beast::string_view path);

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string path_cat(boost::beast::string_view base, boost::beast::string_view path);

// Handle the HTTP(S) request
void handle_request(std::string&& from_addr, beast_request&& req, response_queue& resp_queue);

// Handle the WebSocket traffic
void handle_websocket_request();
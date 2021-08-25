#pragma once

class http_request;
class http_response;

namespace authentication
{
	void on_login(const http_request& req, http_response& resp);
}
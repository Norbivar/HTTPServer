#pragma once

class http_request;
class http_response;

namespace authentication
{
	void request_login(const http_request& req, http_response& resp);
	void request_register(const http_request& req, http_response& resp);


	void test_session(const http_request& req, http_response& resp);
}
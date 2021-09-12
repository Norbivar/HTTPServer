#include "auth.hpp"

#include <Logging>

#include "../http_request.hpp"
#include "../http_response.hpp"
#include "../webserver.hpp"
#include "../session_tracker.hpp"
#include "../database/sql/sql_manager.hpp"
#include "../database/mappers/accounts_mapper.hpp"
#include "../libs/sha256.hpp"

namespace
{
	bool validate_username(const std::string& username)
	{
		return !username.empty();
	}
	bool validate_password(const std::string& password)
	{
		return !password.empty();
	}
	bool validate_email(const std::string& email)
	{
		return !email.empty();
	}
}

void authentication::request_login(const http_request& req, http_response& resp)
{
	if (!req.sid.empty())
	{
		theLog->warn("Login request with session ID skipped.");
		resp.response_code(boost::beast::http::status::internal_server_error);
		return;
	}

	const auto user = req.get<std::string>("user");
	const auto pass = sha256(req.get<std::string>("pass"));

	if (user.empty() || pass.empty())
	{
		resp["error_message"] = "Missing username/password!";
		resp.response_code(boost::beast::http::status::internal_server_error);
		return;
	}

	theLog->info("Login request received '{}'",user);

	auto db = theServer.get_sql_manager().acquire_handle();

	accounts_mapper::filter_t filter;
	filter.username = user;
	filter.password = pass;

	const auto accounts = accounts_mapper::get(db, filter);
	if (accounts.size() != 1)
	{
		resp["error_message"] = "Invalid username/password!";
		resp.response_code(boost::beast::http::status::internal_server_error);
		return;
	}
	const auto& account = accounts.front();

	auto& session_tracker = theServer.get_session_tracker();
	const auto [exists, it] = session_tracker.find_by_account_id(account.id);
	if (exists)
	{
		theLog->warn("Duplicate session request for account ID {}.", account.id);
		resp.response_code(boost::beast::http::status::internal_server_error);
		resp["error_message"] = "Already active session.";
		return;
	}

	theLog->info("Creating session for Account ID {}...", account.id);
	const auto [success, new_it] = session_tracker.create_new_session(account.id);
	theLog->info("Session creation {}.", success ? "succeeded" : "failed");

	if (success)
	{
		new_it->session->modify([](auto& session) {
			session.session_creation_time = std::chrono::system_clock::now();
		});
		resp["sid"] = new_it->session_id;
	}
}

void authentication::request_register(const http_request& req, http_response& resp)
{
	if (!req.sid.empty())
	{
		theLog->warn("Register request with session ID skipped.");
		return;
	}

	const auto user  = req.get<std::string>("user");
	const auto pass  = req.get<std::string>("pass");
	const auto email = req.get<std::string>("email");

	if (!validate_username(user) || !validate_password(pass) || !validate_email(email))
	{ 
		resp.response_code(boost::beast::http::status::internal_server_error);
		resp["error_message"] = "Invalid user data!";
		return;
	}

	auto db = theServer.get_sql_manager().acquire_handle();

	accounts_mapper::filter_t filter;
	filter.OR = true;
	filter.username = user;
	filter.email = email;

	const auto accounts = accounts_mapper::get(db, filter);
	if (!accounts.empty())
	{
		resp.response_code(boost::beast::http::status::internal_server_error);
		resp["error_message"] = "Already taken username/email!";
		return;
	}

	account_element new_account;
	new_account.username = user;
	new_account.password = sha256(pass);
	new_account.email = email;
	new_account.creationtime = std::chrono::system_clock::now();

	if (!accounts_mapper::insert(db, new_account))
	{
		resp["error_message"] = "Invalid username/password!";
		resp.response_code(boost::beast::http::status::internal_server_error);
		return;
	}
	theLog->info("New account created: {}", user);
}

void authentication::test_session(const http_request& req, http_response& resp)
{
	theLog->info("heyho!");
	req.session->acquire()->name;
}
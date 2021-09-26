#include "auth.hpp"

#include <Logging>

#include "../http_request.hpp"
#include "../http_response.hpp"
#include "../webserver.hpp"
#include "../session_tracker.hpp"
#include "../database/sql/sql_manager.hpp"
#include "../database/mappers/accounts_mapper.hpp"
#include "../libs/sha256.hpp"
#include "../libs/format.hpp"

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
		throw std::invalid_argument{"Already logged in."};

	const auto user = req.get<std::string>("user");
	const auto pass = sha256(req.get<std::string>("pass"));
	const auto obliterate_sessions = req.get<boost::optional<bool>>("obliterate_sessions");

	if (user.empty() || pass.empty())
		throw std::invalid_argument{"Missing username/password!"};

	theLog->info("Login request received '{}'", user);

	auto db = theServer.get_sql_manager().acquire_handle();

	accounts_mapper::filter_t filter;
	filter.username = user;
	filter.password = pass;

	const auto accounts = accounts_mapper::get(db, filter);
	if (accounts.size() != 1)
		throw std::invalid_argument{"Invalid username/password!"};

	const auto& account = accounts.front();

	auto& session_tracker = theServer.get_session_tracker();
	const auto [exists, it] = session_tracker.find_by_account_id(account.id);
	if (exists)
	{
		if (obliterate_sessions)
		{
			if (*obliterate_sessions)
			{
				const auto destroyed = session_tracker.obliterate_sessions_by_account_id(account.id);
				theLog->info("Obliterated {} sessions with account id {}.", destroyed, account.id);
			}
		}
		else
		{
			resp["exists"] = true;
			return;
		}
	}

	const auto [success, new_it] = session_tracker.create_new_session(account.id);
	theLog->info("Session creation {}. Account ID: {} | SID: {}", success ? "succeeded" : "failed", new_it->account_id, new_it->session_id);

	if (success)
	{
		new_it->session->session_creation_time = std::chrono::system_clock::now();
		new_it->session->ip_address = req.address;
		resp.set_cookie(format_string("SID=%1%;", new_it->session_id));
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
		throw std::invalid_argument{"Invalid username/password!"};

	auto db = theServer.get_sql_manager().acquire_handle();

	accounts_mapper::filter_t filter;
	filter.OR = true;
	filter.username = user;
	filter.email = email;

	const auto accounts = accounts_mapper::get(db, filter);
	if (!accounts.empty())
		throw std::invalid_argument{"Already taken username/email!"};

	account_element new_account;
	new_account.username = user;
	new_account.password = sha256(pass);
	new_account.email = email;
	new_account.creationtime = std::chrono::system_clock::now();

	if (!accounts_mapper::insert(db, new_account))
		throw std::invalid_argument{"Invalid username/password!"};

	theLog->info("New account created: {}", user);
}

void authentication::test_session(const http_request& req, http_response& resp)
{
	theLog->info("heyho!");
}
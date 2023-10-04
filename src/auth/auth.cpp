#include "auth.hpp"

#include <Logging>

#include "../base/http_request.hpp"
#include "../base/http_response.hpp"
#include "../WebServer.hpp"
#include "../session_tracker.hpp"
#include "../database/sql/sql_manager.hpp"
#include "../database/mappers/accounts_mapper.hpp"

namespace
{
	bool validate_username(const std::string& username)
	{
		return !username.empty();
	}
	bool validate_password(const std::string& password)
	{
		return password.length() >= 64;
	}
	bool validate_email(const std::string& email)
	{
		const std::regex pattern("(\\w+)(\\.|_)?(\\w*)@(\\w+)(\\.(\\w+))+");
		return std::regex_match(email, pattern);
	}
}

void authentication::request_login(const http_request& req, http_response& resp)
{
	if (!req.sid.empty())
		throw std::invalid_argument{ "Already logged in." };

	const auto user = req.get<std::string>("user");
	const auto pass_encoded = req.get<std::string>("pass");
	const auto obliterate_sessions = req.get<boost::optional<bool>>("obliterate_sessions");

	if (user.empty() || pass_encoded.empty())
		throw std::invalid_argument{ "Missing username/password!" };

	theLog->info("Login request received '{}'", user);

	const auto db = theServer.get_sql_manager().acquire_handle();

	accounts_mapper::filter_t filter;
	filter.username = user;
	filter.password = pass_encoded;

	const auto accounts = accounts_mapper::get(db, filter);
	if (accounts.size() != 1)
		throw std::invalid_argument{ "Invalid username/password!" };

	const auto& account = accounts.front();

	auto& session_tracker = theServer.get_session_tracker();
	const auto [exists, it] = session_tracker.find_by_account_id(account.id);
	if (exists)
	{
		if (obliterate_sessions.get_value_or(false))
		{
			const auto destroyed = session_tracker.obliterate_sessions_by_account_id(account.id);
			theLog->info("Obliterated {} sessions with account id {}.", destroyed, account.id);
		}
		else
		{
			resp["exists"] = true;
			return;
		}
	}

	const auto [success, new_it] = session_tracker.create_new_session(req.address, account.id);
	theLog->info("Account ID {} logged in successfully.", new_it->account_id);

	if (success)
		resp.set_cookie(fmt::format("SID={};", new_it->session_id));
}

void authentication::request_register(const http_request& req, http_response& resp)
{
	if (!req.sid.empty())
	{
		theLog->warn("Register request with session ID skipped.");
		return;
	}

	const auto user = req.get<std::string>("user");
	const auto pass = req.get<std::string>("pass"); // should already be in SHA-256
	const auto email = req.get<std::string>("email");

	if (!validate_username(user) || !validate_password(pass) || !validate_email(email))
		throw std::invalid_argument{ "Invalid form of username/password/email!" };

	auto db = theServer.get_sql_manager().acquire_handle();

	accounts_mapper::filter_t filter;
	filter.OR = true;
	filter.username = user;
	filter.email = email;

	const auto accounts = accounts_mapper::get(db, filter);
	if (!accounts.empty())
		throw std::invalid_argument{ "Already taken username/email!" };

	account_element new_account;
	new_account.username = user;
	new_account.password = pass;
	new_account.email = email;
	new_account.creationtime = std::chrono::system_clock::now();

	if (!accounts_mapper::insert(db, new_account))
		throw std::invalid_argument{ "Invalid username/password!" };

	theLog->info("New account created: {}", user);
}

struct testermester
{
	int i = 1;
	std::string s = "s";
};

void to_json(nlohmann::json& j, const testermester& p) {
	j = nlohmann::json{ {"i", p.i}, {"s", p.s} };
}

void from_json(const nlohmann::json& j, testermester& p) {
	j.at("i").get_to(p.i);
	j.at("s").get_to(p.s);
}

void authentication::test_session(const http_request& req, http_response& resp)
{
	/*theLog->info("heyho!");
	theLog->info("Test session : {}", req.session->acquire()->session_id);

	auto email = req.get<testermester>("obj");
	theLog->info("VN: after read: [ i : {} | s : '{}' ]", email.i, email.s);

	email.i = 55;
	email.s = "kerek";

	theLog->info("VN: after mod: [ i : {} | s : '{}' ]", email.i, email.s);
	resp = email;*/

	theLog->error("VN: GETTING DB");
	auto db = theServer.get_sql_manager().acquire_handle();
	theLog->error("VN: GOT DB");

	std::this_thread::sleep_for(std::chrono::seconds{ 5 });
	theLog->error("VN: DONE");

}
#include "auth.hpp"

#include <Logging>

#include "../base/http_request.hpp"
#include "../base/http_response.hpp"
#include "../base/exceptions.hpp"
#include "../WebServer.hpp"
#include "../session_tracker.hpp"
#include "../database/sql/sql_manager.hpp"
#include "../database/mappers/accounts_mapper.hpp"
#include "../websocket_tracker.hpp"

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
	auto& session_tracker = theServer.get_session_tracker();
	
	if (!req.sid.empty())
	{
		const auto [exists, _] = session_tracker.find_by_session_id(req.sid);
		if (exists)
			throw user_invalid_argument{ "Already logged in." };
	}

	const auto user = req.get<std::string>("user");
	const auto pass_encoded = req.get<std::string>("pass");
	const auto obliterate_sessions = req.get<boost::optional<bool>>("obliterate_sessions");

	if (user.empty() || pass_encoded.empty())
		throw user_invalid_argument{ "Missing username/password!" };

	theLog->info("Login request received '{}'", user);

	const auto db = theServer.get_sql_manager().acquire_handle();

	accounts_mapper::filter_t filter;
	filter.username = user;
	filter.password = pass_encoded;

	const auto accounts = accounts_mapper::get(db, filter);
	if (accounts.size() != 1)
		throw user_invalid_argument{ "Invalid username/password!" };

	const auto& account = accounts.front();

	const auto [success, new_it] = session_tracker.create_new_session(req.address, account.id, obliterate_sessions.get_value_or(false));
	if (success) 
	{
		theLog->info("Account ID {} logged in successfully.", new_it->account_id);
		resp.set_cookie(fmt::format("SID={};", new_it->session_id));
	}
	else
	{
		theLog->error("Account ID {} failed to log in.", new_it->account_id);
		throw user_invalid_argument{ "Invalid username/password!" };
	}
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

void authentication::request_websocket(const http_request& req, http_response& resp)
{
	const auto [found, _] = theServer.get_session_tracker().find_by_session_id(req.sid);
	if (found)
	{
		auto new_handshake_code = theServer.get_websocket_tracker().get_new_handshake_code(req.sid);
		if (!new_handshake_code.empty())
			resp["handshake"] = new_handshake_code;
	}
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

#include "websocket_tracker.hpp"
#include "base/websocket_session.hpp"

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


	theLog->error("VN: Getting websocket...");
	const auto sockets = theServer.get_websocket_tracker().find_sockets(req.sid);

	theLog->error("VN: Found {} sockets", sockets.size());
	if (sockets.size()) {
		theLog->error("VN: Sending message through first socket");

		sockets[0]->send("asd");
	}
}
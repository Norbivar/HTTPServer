#include "auth.hpp"

#include <Logging>

#include "../http_request.hpp"
#include "../http_response.hpp"
#include "../webserver.hpp"

void authentication::on_login(const http_request& req, http_response& resp)
{
	if (!req.sid.empty())
	{
		theLog->warn("Login request with session ID skipped.");
		return;
	}

	const auto user = req.get<std::string>("user");
	const auto pass = req.get<std::string>("pass");

	theLog->info("Login request received : username: '{}' | password: '{}'.",
		user,
		pass);

	auto& session_tracker = theServer.get_session_tracker();

	const auto account_id = 1;
	const auto [exists, it] = session_tracker.find_by_account_id(account_id);
	if (exists)
	{
		theLog->warn("Duplicate session request for account ID {}.", account_id);
		return;
	}

	theLog->info("Creating session for Account ID {}...", account_id);
	const auto [success, new_it] = session_tracker.create_new_session(account_id);
	theLog->info("Session creation {}.", success ? "succeeded" : "failed");

	if (success)
	{
		resp["sid"] = new_it->session_id;
	}
}
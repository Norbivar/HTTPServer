#include "auth.hpp"

#include <json.hpp>

#include <Logging>
#include "../webserver.hpp"

void authentication::on_login(const std::string& ssl_id, const nlohmann::json& req, nlohmann::json& resp)
{
	if (ssl_id.empty())
	{
		theLog->warn("Empty SSL ID login request");
		return;
	}

	theLog->info("Login request for SSL ID '{}'.", ssl_id);

	auto& session_tracker = theServer.get_session_tracker();
	const auto [exists, it] = session_tracker.find_by_ssl(ssl_id);
	if (exists)
	{
		theLog->warn("Duplicate login request SSL ID '{}'. Session ID : '{}'", ssl_id, it->session_id);
		return;
	}

	const auto user = json_get<std::string>(req, "user");
	const auto pass = json_get<std::string>(req, "pass");

	std::uint32_t account_id = 1; //PLACEHOLDER ACCOUNT ID
	// get account id... from database user+pass

	theLog->info("Creating session for Account ID {}...", account_id);
	const auto [success, new_it] = session_tracker.create_new_session(ssl_id, 1); //PLACEHOLDER ACCOUNT ID
	theLog->info("Session creation {}.", success ? "succeeded" : "failed");

	if (success)
	{
		resp.emplace("sid", new_it->session_id);
	}
}
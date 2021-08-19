#include "auth.hpp"

#include <boost/property_tree/ptree.hpp>

#include <Logging>
#include "../webserver.hpp"

void authentication::on_login(const std::string& ssl_id, const nlohmann::json& arguments, nlohmann::json& resp)
{
	if (ssl_id.empty())
	{
		theLog->warn("Empty SSL ID login request");
		return;
	}

	theLog->error("Login request for SSL ID '{}'", ssl_id);

	auto& session_tracker = theServer.get_session_tracker();
	const auto [exists, it] = session_tracker.find_by_ssl(ssl_id);
	if (exists)
	{
		theLog->warn("Duplicate login request SSL ID '{}'. Session ID : '{}'", ssl_id, it->session_id);
		return;
	}

	//theLog->info("Creating session.");
	//session_tracker.get_new_session(ssl_id, 1);
}
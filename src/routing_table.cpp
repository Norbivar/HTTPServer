#include "routing_table.hpp"

#include <chrono>
#include <thread>

#include <Logging>
#include <json.hpp>

#include "session_tracker.hpp"
#include "auth/auth.hpp"

void routing_table::register_all()
{
	using method = boost::beast::http::verb;

	register_path(method::post, "/login", authentication::on_login);

	register_path(method::post, "/test", [](const nlohmann::json& req, nlohmann::json& resp) {
		//std::this_thread::sleep_for(std::chrono::seconds(5));

		const auto user = json_get<std::string>(req, "user");
		const auto pass = json_get_optional<std::string>(req, "pass");
		theLog->error("Received that mofo GET test request! {} + {}", user, pass.get_value_or(""));

		resp.emplace("ketszaz", "pluszafa");
	});

	register_path(method::post, "/test_get_session", [](session_info& info, const nlohmann::json& req, nlohmann::json& resp) {
		resp.emplace("nev", info.acquire().data().name);
	});

	register_path(method::post, "/test_deny", [](const nlohmann::json& arguments, nlohmann::json& resp) { theLog->error("Received that mofo test deny request!"); }, []() { return false; });

}

void routing_table::register_path(boost::beast::http::verb verb, std::string&& path, const routing_handler& handler, const routing_access_predicate& pred)
{
	auto& verb_table_it = m_tables[verb];
	verb_table_it.emplace(std::move(path), routing_entry{ handler, pred });
}

std::pair<bool, routing_table::table_t::const_iterator> routing_table::lookup(boost::beast::http::verb verb, const std::string& path) const
{
	std::pair<bool, routing_table::table_t::const_iterator> entry{ false, nullptr };

	const auto verb_table_it = m_tables.find(verb);
	if (verb_table_it == m_tables.end())
		return entry;

	const auto it = verb_table_it->second.find(path);
	if (it != verb_table_it->second.end())
	{
		entry.second = it;
		entry.first = true;
	}
	return entry;
}

void routing_table::print_stats() const
{
	theLog->info("	Routing Table entries by methods:");
	for (const auto& [v, t] : m_tables)
		theLog->info("		{} : {} entries", to_string(v), t.size());
}
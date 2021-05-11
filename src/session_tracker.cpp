#include "session_tracker.hpp"

#include <exception>
#include <Logging>

std::string generate_unique_http_session_id()
{
	return "qwertzuiopasdfghjkl";
}

std::pair<bool, session_map::iterator> session_tracker::get_new_session(const std::string& ssl_id, std::uint32_t account_id)
{
	std::unique_lock lock{m_mutex};

	const auto& [found, _] = find_by_session_impl(ssl_id);
	if (found)
		throw std::logic_error{"get_new_session with duplicate ssl id NOT IMPLEMENTED!"};

	bool is_unique_session = false;
	std::string new_session_id;
	while (!is_unique_session)
	{
		new_session_id = generate_unique_http_session_id();
		const auto& [exists, __] = find_by_session_impl(new_session_id);
		is_unique_session = !exists;
	}

	auto [it, inserted] = m_session_container.emplace(new_session_id, ssl_id);
	if (inserted)
	{
		it->info = std::make_unique<session_info>(); // TODO: add session from DB
		theLog->info("Session created. ID: {} (SSL ID: {})", ssl_id, ssl_id);
	}
	return {inserted, it};
}

std::pair<bool, session_map::nth_index<0>::type::iterator> session_tracker::find_by_session(const std::string& sid) const
{
	std::shared_lock lock{m_mutex};
	return find_by_session_impl(sid);
}

std::pair<bool, session_map::nth_index<1>::type::iterator> session_tracker::find_by_ssl(const std::string& ssl_id) const
{
	std::shared_lock lock{m_mutex};
	return find_by_ssl_impl(ssl_id);
}


std::pair<bool, session_map::nth_index<0>::type::iterator> session_tracker::find_by_session_impl(const std::string& sid) const
{
	std::pair<bool, session_map::nth_index<0>::type::iterator> result {false, nullptr};
	const auto& cont = m_session_container.get<0>();
	const auto it = cont.find(sid);
	
	if (it != cont.end())
	{
		result.first = true;
		result.second = it;
	}
	return result;
}

std::pair<bool, session_map::nth_index<1>::type::iterator> session_tracker::find_by_ssl_impl(const std::string& ssl_id) const
{
	std::pair<bool, session_map::nth_index<1>::type::iterator> result{ false, nullptr };
	const auto& cont = m_session_container.get<1>();
	const auto it = cont.find(ssl_id);
	if (it != cont.end())
	{
		result.first = true;
		result.second = it;
	}
	return result;
}
#include "session_tracker.hpp"

#include <exception>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include <Logging>

#include "session_info.hpp"

constexpr std::uint32_t sid_length = 64;

std::string generate_unique_http_session_id()
{
	constexpr std::array<char, 63> chars =
	{
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"1234567890"
	};

	std::string sid;
	sid.reserve(sid_length);

	boost::random::random_device rng;
	boost::random::uniform_int_distribution<> index_dist(0, chars.size() - 1);
	for (int i = 0; i < sid_length; ++i)
		sid.push_back(chars[index_dist(rng)]);

	return sid;
}

std::pair<bool, session_map::iterator> session_tracker::create_new_session(const std::string& ssl_id, const std::uint32_t account_id)
{
	std::unique_lock lock{m_mutex};

	const auto& [found, _] = find_by_ssl_impl(ssl_id); // SSL ID does not really work :( remove this foul creature, and use only session ID I guess
	if (found)
		throw std::logic_error{"create_new_session with duplicate ssl id NOT IMPLEMENTED!"};

	bool is_unique_session = false;
	std::string new_session_id;
	while (!is_unique_session)
	{
		new_session_id = generate_unique_http_session_id();
		const auto& [exists, __] = find_by_session_impl(new_session_id);
		is_unique_session = !exists;
	}

	session_keys session_key_obj{ new_session_id, ssl_id };
	session_key_obj.info = std::make_unique<session_info>(); // TODO: add session from DB

	auto [it, inserted] = m_session_container.emplace(std::move(session_key_obj));
	return {inserted, it};
}

std::pair<bool, session_map::nth_index<0>::type::iterator> session_tracker::find_by_session(const std::string& sid) const
{
	std::shared_lock lock{m_mutex};
	return find_by_session_impl(sid);
}

std::pair<bool, session_map::nth_index<1>::type::iterator> session_tracker::find_by_ssl(const std::string& ssl_id) const
{
	theLog->error("Looking up SSL ID '{}', amongst {} entries!", ssl_id, m_session_container.size());
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
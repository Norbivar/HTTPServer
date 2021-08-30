#include "session_tracker.hpp"

#include <exception>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>

#include <Logging>

#include "session_info.hpp"

constexpr auto sid_length = 64;
constexpr auto session_id_generation_max_attempts = 10;

id::session generate_unique_http_session_id()
{
	constexpr char chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";

	id::session sid;
	sid.reserve(sid_length);

	boost::random::random_device rng;
	boost::random::uniform_int_distribution<> index_dist(0, sizeof(chars) - 2); // last one is a \0
	for (int i = 0; i < sid_length; ++i)
		sid.push_back(chars[index_dist(rng)]);

	return sid;
}

std::pair<bool, session_map::iterator> session_tracker::create_new_session(const id::account accounnt_id)
{
	std::unique_lock lock{m_mutex};

	std::uint8_t tries = 0;
	auto is_unique_session = false;
	id::session new_session_id;
	while (!is_unique_session)
	{
		if (tries == session_id_generation_max_attempts)
		{
			theLog->error("Hit maximum number of session id generation for account id {}", accounnt_id);
			break;
		}

		new_session_id = generate_unique_http_session_id();
		const auto& [exists, __] = find_by_session_id_impl(new_session_id);
		is_unique_session = !exists;
		
		++tries;
	}

	session_keys session_key_obj{ new_session_id, accounnt_id };
	session_key_obj.info = std::make_unique<session_info>(); // TODO: add session from DB

	auto [it, inserted] = m_session_container.emplace(std::move(session_key_obj));
	return {inserted, it};
}

bool session_tracker::obliterate_session(const id::session& sid)
{
	std::unique_lock lock{m_mutex};
	auto [found, it] = find_by_session_id_impl(sid);
	if (found)
	{
		m_session_container.erase(it);
		return true;
	}
	return false;
}

std::pair<bool, session_map::nth_index<0>::type::iterator> session_tracker::find_by_session_id(const id::session& sid) const
{
	std::shared_lock lock{m_mutex};
	return find_by_session_id_impl(sid);
}

std::pair<bool, session_map::nth_index<1>::type::iterator> session_tracker::find_by_account_id(const id::account account_id) const
{
	std::shared_lock lock{m_mutex};
	return find_by_account_id_impl(account_id);
}

std::pair<bool, session_map::nth_index<0>::type::iterator> session_tracker::find_by_session_id_impl(const id::session& sid) const
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

std::pair<bool, session_map::nth_index<1>::type::iterator> session_tracker::find_by_account_id_impl(const id::account account_id) const
{
	std::pair<bool, session_map::nth_index<1>::type::iterator> result{ false, nullptr };
	const auto& cont = m_session_container.get<1>();
	const auto it = cont.find(account_id);
	if (it != cont.end())
	{
		result.first = true;
		result.second = it;
	}
	return result;
}
#include "session_tracker.hpp"

#include <exception>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/range/algorithm_ext/erase.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/transformed.hpp>

#include <Logging>

#include "webserver.hpp"
#include "database/mappers/sessions_mapper.hpp"
#include "database/sql/sql_manager.hpp"
#include "database/sql/sql_handle.hpp"

constexpr auto sid_length = 64; // If changed, the database needs to change too
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

session_tracker::session_tracker(std::unique_ptr<sql_manager>& sqlm)
{
	if (!sqlm)
	{
		theLog->error("Session tracker got invalid sql manager! Could not load sessions");
		return;
	}

	auto handle = sqlm->acquire_handle();
	std::unique_lock lock{ m_mutex }; // This is probably not needed because it's the constructor, but hey...

	auto all_saved_sessions = sessions_mapper::get_all(handle);
	for (auto& saved_sess : all_saved_sessions)
		emplace_session(std::move(saved_sess));

	theLog->info("Loaded {} sessions. ", m_session_container.size());
}

session_tracker::~session_tracker()
{
	std::shared_lock lock{ m_mutex };

	const auto datas = m_session_container
		| boost::adaptors::transformed([](const auto& s) { return s.session->acquire(); }) // TODO: force-lock / ignore if cannot take?
		| boost::adaptors::filtered([](const auto& s) { return !s->deactivated; })
		| boost::adaptors::transformed([](const auto& s) {return s.data(); });

	theLog->info("Saving {} sessions.", std::distance(datas.begin(), datas.end()));

	{
		auto handle = theServer.get_sql_manager().acquire_handle();
		sessions_mapper::insert(handle, to_sql(handle, datas));
	}

	theLog->info("Session tracker shut down.");
}

std::pair<bool, session_map::iterator> session_tracker::create_new_session(const std::string& ip_address, const id::account account_id, bool delete_other_for_account)
{
	std::unique_lock lock{ m_mutex };

	std::uint8_t tries = 0;
	auto is_unique_session = false;
	id::session new_session_id;
	while (!is_unique_session)
	{
		if (tries == session_id_generation_max_attempts)
		{
			new_session_id.clear();
			theLog->error("Hit maximum number of session id generation for account id {}", account_id);
			break;
		}

		new_session_id = generate_unique_http_session_id();
		const auto& [exists, _] = find_by_session_id_impl(new_session_id);
		is_unique_session = !exists;

		++tries;
	}

	if (new_session_id.empty())
		return { false, nullptr };

	if (delete_other_for_account)
		m_session_container.get<1>().erase(account_id);

	session_element new_sess{ new_session_id, account_id };
	new_sess.session_creation_time = std::chrono::system_clock::now();
	new_sess.ip_address = ip_address;

	return emplace_session(std::move(new_sess));
}

std::size_t session_tracker::obliterate_sessions_by_account_id(const id::account& sid)
{
	std::unique_lock lock{ m_mutex };
	return m_session_container.get<1>().erase(sid);
}

std::pair<bool, session_map::nth_index<0>::type::iterator> session_tracker::find_by_session_id(const id::session& sid) const
{
	std::shared_lock lock{ m_mutex };
	return find_by_session_id_impl(sid);
}

std::pair<bool, session_map::nth_index<1>::type::iterator> session_tracker::find_by_account_id(const id::account account_id) const
{
	std::shared_lock lock{ m_mutex };
	return find_by_account_id_impl(account_id);
}

std::pair<bool, session_map::nth_index<0>::type::iterator> session_tracker::find_by_session_id_impl(const id::session& sid) const
{
	std::pair<bool, session_map::nth_index<0>::type::iterator> result{ false, nullptr };
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

std::pair<bool, session_map::iterator> session_tracker::emplace_session(session_element&& session)
{
	auto sid = session.session_id;
	auto account_id = session.account_id;

	auto new_session_ptr = std::make_shared<threadsafe::element<session_element>>(std::move(session));

	auto [it, inserted] = m_session_container.emplace(std::move(sid), std::move(account_id), std::move(new_session_ptr));
	return { inserted, it };
}
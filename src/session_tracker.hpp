#pragma once

#include <shared_mutex>
#include <string>
#include <memory>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/member.hpp>

#include "id_types.hpp"
#include "threadsafe.hpp"
#include "database/session_element.hpp"

struct session_keys
{
	session_keys(const id::session& sid, const id::account& sslid, std::shared_ptr<threadsafe::element<session_element>>&& sess) :
		session_id{sid},
		account_id{sslid},
		session{std::move(sess)}
	{}
	// Indexes:
	const id::session session_id;
	const id::account account_id;

	std::shared_ptr<threadsafe::element<session_element>> session; 
};

using session_map = boost::multi_index::multi_index_container<
	session_keys,
	boost::multi_index::indexed_by<
		boost::multi_index::hashed_unique<
			BOOST_MULTI_INDEX_MEMBER(session_keys, const id::session, session_id)
		>,
		boost::multi_index::ordered_non_unique<
			BOOST_MULTI_INDEX_MEMBER(session_keys, const id::account, account_id)
		>
	>
>;

class session_tracker
{
public:
	session_tracker(std::unique_ptr<class sql_manager>& sqlm);
	~session_tracker();

	using session_by_sid_iterator = session_map::nth_index<0>::type::iterator;
	using session_by_account_iterator = session_map::nth_index<1>::type::iterator;

	std::pair<bool, session_map::iterator> create_new_session(const id::account account_id, bool delete_other_for_account = false);
	std::size_t obliterate_sessions_by_account_id(const id::account& sid);

	std::pair<bool, session_by_sid_iterator> find_by_session_id(const id::session& sid) const;
	std::pair<bool, session_by_account_iterator> find_by_account_id(const id::account account_id) const;

private:
	session_map m_session_container;

	std::pair<bool, session_by_sid_iterator> find_by_session_id_impl(const id::session& sid) const;
	std::pair<bool, session_by_account_iterator> find_by_account_id_impl(const id::account account_id) const;

	std::pair<bool, session_map::iterator> emplace_session(const id::session& sid, const id::account account_id);

	mutable std::shared_mutex m_mutex;
};
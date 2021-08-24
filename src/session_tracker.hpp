#pragma once

#include <shared_mutex>
#include <string>
#include <memory>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>

#include "id_types.hpp"
#include "session_info.hpp"

struct session_keys
{
	session_keys(const std::string& sid, const id::account& sslid) : 
		session_id{sid},
		account_id{sslid}
	{}
	// Indexes:
	const std::string session_id;
	const id::account account_id;

	mutable std::shared_ptr<session_info> info; // atomic shared ptr?
};

using session_map = boost::multi_index::multi_index_container<
	session_keys,
	boost::multi_index::indexed_by<
		boost::multi_index::hashed_unique<
			BOOST_MULTI_INDEX_MEMBER(session_keys, const std::string, session_id)
		>,
		boost::multi_index::hashed_unique<
			BOOST_MULTI_INDEX_MEMBER(session_keys, const id::account, account_id)
		>
	>
>;

class session_tracker
{
public:
	std::pair<bool, session_map::iterator> create_new_session(const id::account account_id);
	bool obliterate_session(const id::session& sid);

	std::pair<bool, session_map::nth_index<0>::type::iterator> find_by_session_id(const id::session& sid) const;
	std::pair<bool, session_map::nth_index<1>::type::iterator> find_by_account_id(const id::account account_id) const;

private:
	session_map m_session_container;

	std::pair<bool, session_map::nth_index<0>::type::iterator> find_by_session_id_impl(const id::session& sid) const;
	std::pair<bool, session_map::nth_index<1>::type::iterator> find_by_account_id_impl(const id::account account_id) const;

	mutable std::shared_mutex m_mutex;
};
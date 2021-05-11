#pragma once

#include <shared_mutex>
#include <string>
#include <memory>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>

#include "session_info.hpp"

struct session_keys
{
	session_keys(const std::string& sid, const std::string& sslid) : 
		session_id{sid},
		ssl_id{sslid}
	{}
	// Indexes:
	const std::string session_id;
	const std::string ssl_id;

	mutable std::unique_ptr<session_info> info;
};

using session_map = boost::multi_index::multi_index_container<
	session_keys,
	boost::multi_index::indexed_by<
		boost::multi_index::hashed_unique<
			BOOST_MULTI_INDEX_MEMBER(session_keys, const std::string, session_id)
		>,
		boost::multi_index::hashed_unique<
			BOOST_MULTI_INDEX_MEMBER(session_keys, const std::string, ssl_id)
		>
	>
>;

class session_tracker
{
public:
	std::pair<bool, session_map::iterator> get_new_session(const std::string& ssl_id, std::uint32_t account_id);
	std::pair<bool, session_map::nth_index<0>::type::iterator> find_by_session(const std::string& sid) const;
	std::pair<bool, session_map::nth_index<1>::type::iterator> find_by_ssl(const std::string& ssl_id) const;

private:
	session_map m_session_container;

	std::pair<bool, session_map::nth_index<0>::type::iterator> find_by_session_impl(const std::string& sid) const;
	std::pair<bool, session_map::nth_index<1>::type::iterator> find_by_ssl_impl(const std::string& ssl_id) const;

	mutable std::shared_mutex m_mutex;
};
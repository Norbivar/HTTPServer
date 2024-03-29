#pragma once

#include <chrono>
#include <string>

#include "../id_types.hpp"

class sql_handle;

// The database part of a session. These are the data that are persistent for each session. 
struct session_element
{
	const id::account account_id; 
	const id::session session_id;
	std::chrono::system_clock::time_point session_creation_time{};
	std::chrono::system_clock::time_point last_request_time{};
	bool deactivated{ false };
	std::string ip_address{};

	static std::string to_sql_string(const session_element& elem, const sql_handle& db);

private:
	friend class session_tracker;
	friend class sessions_mapper;

	session_element(const id::session& sid, const id::account acc_id) :
		session_id{ sid },
		account_id{ acc_id }
	{}
};
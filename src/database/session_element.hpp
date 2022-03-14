#pragma once

#include <chrono>
#include <string>

#include "../id_types.hpp"

class sql_handle;

struct session_element
{
	session_element(const id::session& sid, const id::account acc_id) :
		session_id{ sid },
		account_id{ acc_id }
	{}

	const id::account account_id; 
	const id::session session_id;
	std::chrono::system_clock::time_point session_creation_time{};
	std::chrono::system_clock::time_point last_request_time{};
	bool deactivated{ false };
	std::string ip_address{};

	static std::string to_sql_string(const sql_handle& db, const session_element& elem);
};
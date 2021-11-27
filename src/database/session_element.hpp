#pragma once

#include <chrono>

#include "../id_types.hpp"

struct session_element
{
	session_element(const id::session& sid, const id::account acc_id) :
		session_id{ sid },
		account_id{ acc_id }
	{}

	const id::account account_id; 
	const id::session session_id;
	std::chrono::system_clock::time_point session_creation_time{};
	bool deactivated{ false };
	std::string ip_address{};
};
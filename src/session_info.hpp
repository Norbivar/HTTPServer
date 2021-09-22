#pragma once

#include <shared_mutex>
#include <string>
#include <chrono>
#include "id_types.hpp"

struct session_data
{
	session_data(const id::session& sid, const id::account acc_id) :
		session_id{sid},
		account_id{acc_id}
	{}
	const id::session session_id;
	const id::account account_id;
	std::chrono::system_clock::time_point session_creation_time{};
	bool deactivated{false};
	std::string ip_address{};
};


/*
class session_info : public std::enable_shared_from_this<session_info>
{
	struct session_read_lock
	{
		session_read_lock(const std::shared_ptr<const session_info> session) : session{session}, lock{session->m_mutex} { }
		const session_data& data() const { return session->m_data; }
		const session_data* operator->() const { return &data(); }

	private:
		const std::shared_ptr<const session_info> session;
		std::shared_lock<std::shared_mutex> lock;
	};

public:
	template<typename T>
	void modify(const T& t) { std::unique_lock<std::shared_mutex> lock(m_mutex); t(m_data); }
	const session_read_lock acquire() const { return session_read_lock{shared_from_this()}; }
	session_data copy() const { std::shared_lock lock{ m_mutex }; return m_data; }

private:
	session_data m_data;
	mutable std::shared_mutex m_mutex;
};*/

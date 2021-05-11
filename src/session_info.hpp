#pragma once

#include <shared_mutex>
#include <string>

class session_info
{
	struct session_data
	{
		std::string name;


	};

	struct session_read_lock
	{
		session_read_lock(const session_info& session) : session{session}, lock{session.m_mutex} { }
		const session_data& data() const { return session.m_data; }
		const session_data* operator->() const { return &data(); }

	private:
		const session_info& session;
		std::shared_lock<std::shared_mutex> lock;
	};

public:
	template<typename T>
	void modify(const T& t) { t(m_data); }
	const session_read_lock acquire() const { return *this; }
	session_data copy() const { std::shared_lock lock {m_mutex}; return m_data; }

private:
	session_data m_data;
	mutable std::shared_mutex m_mutex;
};

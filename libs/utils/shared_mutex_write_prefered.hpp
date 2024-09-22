#pragma once

#include <shared_mutex>
#include <condition_variable>

class shared_mutex_write_prefered
{
public:
	shared_mutex_write_prefered() = default;

	void lock();
	void unlock();
	void lock_shared();
	void unlock_shared();

private:
	void trigger_next();

	std::atomic<uint8_t> writers_waiting{ 0 };

	std::condition_variable read_cv;
	std::condition_variable write_cv;

	mutable std::shared_mutex actual_mutex;
	mutable std::mutex condition_variable_mutex;
};
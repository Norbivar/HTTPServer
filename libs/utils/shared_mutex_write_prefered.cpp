#include "shared_mutex_write_prefered.hpp"

// TODO: Seems buggy, either improve upon the idea or get rid of it

void shared_mutex_write_prefered::trigger_next()
{
	if (writers_waiting)
	{
		write_cv.notify_one();
	}
	else
	{
		read_cv.notify_one();
	}
}

void shared_mutex_write_prefered::lock()
{
	if (!actual_mutex.try_lock())
	{
		++writers_waiting;

		std::unique_lock<std::mutex> wait_lock{ condition_variable_mutex };
		auto write_res = write_cv.wait_for(wait_lock, std::chrono::seconds{30});

		--writers_waiting;

		if (write_res == std::cv_status::timeout) 
			lock(); // Timed out so try again. This is not ideal being in a recursive call, but it should not hit the timeout under most circumstances
	}

	// else we just continue on with the job
}

void shared_mutex_write_prefered::unlock()
{
	actual_mutex.unlock();
	trigger_next();
}

void shared_mutex_write_prefered::lock_shared()
{
	if (writers_waiting || !actual_mutex.try_lock_shared())
	{
		std::unique_lock<std::mutex> wait_lock{ condition_variable_mutex };
		read_cv.wait(wait_lock);
	}

	// carry on
}

void shared_mutex_write_prefered::unlock_shared()
{
	actual_mutex.unlock_shared();
	trigger_next();
}
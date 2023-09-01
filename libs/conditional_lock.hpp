#pragma once

#include <shared_mutex>

template<typename Mutex, typename Pred>
inline std::unique_lock<Mutex> unique_conditional_lock(Mutex& mutex, const Pred& pred)
{
	if (pred())
		return std::unique_lock<Mutex>{mutex} ;
	else
		return {};
}
template<typename Mutex, typename Pred>
inline std::shared_lock<Mutex> shared_conditional_lock(Mutex& mutex, const Pred& pred)
{
	if (pred())
		return std::shared_lock<Mutex>{mutex};
	else
		return {};
}
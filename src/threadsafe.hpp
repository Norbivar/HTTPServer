#pragma once

#include <shared_mutex>

namespace threadsafe
{
	template<typename T>
	class element
	{
		struct element_read_lock
		{
			element_read_lock(const element& elem) : target{ elem }, lock{ elem.m_mutex } { }
			const T& data() const { return target.m_data; }
			const T* operator->() const { return &data(); }

		private:
			const element& target;
			std::shared_lock<std::shared_mutex> lock;
		};

		friend struct element_read_lock;

	public:
		template<typename... Args>
		element(Args&&... args) : m_data(std::forward<Args>(args)...) {}

		template<typename Func>
		void modify(const Func& f) { std::unique_lock<std::shared_mutex> lock(m_mutex); f(m_data); }
		const element_read_lock acquire() const { return element_read_lock{ *this }; }
		T copy() const { std::shared_lock lock{ m_mutex }; return m_data; }

	private:
		T m_data;
		mutable std::shared_mutex m_mutex;
	};

}
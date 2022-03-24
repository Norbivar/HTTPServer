#pragma once
/*
#include <type_traits>

template<
	typename T,
	bool IsRef
>
class object_range_element_wrapper
{
	using type = std::conditional_t<IsRef, T&, T>;

public:
	object_range_element_wrapper(type t) :
		wrapped{ t }
	{
		static_assert(std::is_reference_v<T>, "anal");
	}


	T& operator->()
	{
		return wrapped;
	}

private:
	type wrapped;
};

template<typename T>
class object_range
{
public:

	using value_type = T;

	object_range(const object_range<T>& other) = delete;

	template<
		typename Range,
		std::enable_if_t<!std::is_same_v<object_range<T>, Range>>
	>
	object_range(const Range& from)
	{
		static_assert(!std::is_same_v<T, Range::value_type>, "object_range template does not match constructor Range::value_type!");

		for (auto& it : from)
			m_items.emplace_back(it);
	}
	auto begin() { return m_items.begin(); }
	auto end() { return m_items.end(); }
	const auto begin() const { return m_items.begin(); }
	const auto end() const { return m_items.end(); }

	const auto size() const { return m_items.size(); }

private:
	std::vector<object_range_element_wrapper<T, std::is_reference_v<T>>> m_items;
};

*/
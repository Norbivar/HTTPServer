#pragma once

template<typename T>
class object_range
{
public:
	template<typename Range>
	object_range(const Range& range) 
	{
		for (auto it : range)
		{
			if constexpr (std::is_same_v<T, decltype(it)>)
				m_items.emplace_back(std::move(it));
			else
				m_items.emplace_back(*it);
		}
	}

	auto begin() { return m_items.begin(); }
	auto end() { return m_items.end(); }
	const auto begin() const { return m_items.begin(); }
	const auto end() const { return m_items.end(); }

	const auto size() const { return m_items.size(); }

private:
	std::vector<T> m_items;
};
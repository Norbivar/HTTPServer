#pragma once

namespace traits
{
	// SFINAE test
	template <typename T>
	class [[deprecated]] has_from_json
	{
		typedef char one;
		struct two { char x[2]; };

		template <typename C> static one test(decltype(&C::from_json));
		template <typename C> static two test(...);

	public:
		enum { value = sizeof(test<T>(0)) == sizeof(char) };
	};
}

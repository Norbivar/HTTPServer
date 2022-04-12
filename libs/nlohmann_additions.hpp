#pragma once

namespace nlohmann // small, non-intrusive extension to the json lib
{
	template<typename T>
	T get(const nlohmann::json& json, const std::string& name)
	{
		if constexpr (std::is_assignable<T, boost::none_t>::value)
		{
			if (json.count(name))
			{
				if constexpr (traits::has_from_json<T::value_type>::value)
					return T::value_type::from_json(json.at(name));
				else
					return boost::make_optional(json.at(name).get<T::value_type>()); // if overload not found error is here, it probably means the nlohmann json lib does not have overload -> create from_json
			}
			else return boost::none;
		}
		else
		{
			if constexpr (traits::has_from_json<T>::value)
				return T::from_json(json.at(name));
			else
				return json.at(name).get<T>(); // if overload not found error is here, it probably means the nlohmann json lib does not have overload -> create from_json
		}
	}

	// Const char* forwardor
	template<typename T>
	T get(const nlohmann::json& json, const char* name)
	{
		return nlohmann::get<T>(json, std::string{ name });
	}

	// Just to be able to read directly from req without calling .params() all the time
	template<typename T>
	T get(const http_request& req, const std::string& name)
	{
		return nlohmann::get<T>(req.params(), name);
	}

	template<typename T>
	T get(const http_request& req, const char* name)
	{
		return nlohmann::get<T>(req, std::string{ name });
	}
}

#include <nlohmann/json.hpp>
#include <boost/optional/optional.hpp>

template<typename T>
boost::optional<T> json_get_optional(const nlohmann::json& json, const std::string& name)
{
	if (json.count(name))
		return json[name].get<T>();
	else return boost::none;
}

template<typename T>
T json_get(const nlohmann::json& json, const std::string& name)
{
	return json[name].get<T>();
}
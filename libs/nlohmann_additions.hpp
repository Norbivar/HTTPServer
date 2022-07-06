#pragma once

/*

 ==== from_json / to_json for custom classes example: ====

struct testermester
{
	int i = 1;
	std::string s = "s";
};

void to_json(nlohmann::json& j, const testermester& p) {
	j = nlohmann::json{ {"i", p.i}, {"s", p.s} };
}

void from_json(const nlohmann::json& j, testermester& p) {
	j.at("i").get_to(p.i);
	j.at("s").get_to(p.s);
}

void authentication::test_session(const http_request& req, http_response& resp)
{

	theLog->info("heyho!");
	theLog->info("Test session : {}", req.session->acquire()->session_id);

	auto email = req.get<testermester>("obj");
}
*/


namespace nlohmann // small, non-intrusive extension to the json lib
{
	template<typename T>
	T get(const nlohmann::json& json, const std::string& name)
	{
		if constexpr (std::is_assignable<T, boost::none_t>::value) // Is it a boost::optional<T>?
		{
			using val_type = T::value_type;
			if (json.count(name))
				return boost::make_optional(json.at(name).get<val_type>()); // if overload not found error is here, it probably means the nlohmann json lib does not have overload -> create from_json
			else 
				return boost::none;
		}
		else return json.at(name).get<T>(); // if overload not found error is here, it probably means the nlohmann json lib does not have overload -> create from_json
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

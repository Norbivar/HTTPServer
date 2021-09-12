#pragma once

#include <chrono>
#include "../id_types.hpp"

struct account_element
{
	id::account id{0};
	std::string username{};
	std::string password{}; // not neccessarly filled up when selecting it, but used for insert
	std::string email{};
	std::chrono::system_clock::time_point creationtime{};
};
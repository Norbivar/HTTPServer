#include <stdexcept>
#include <string>

class user_invalid_argument : public std::invalid_argument {
public:
	user_invalid_argument(const std::string& message)
		: std::invalid_argument(message) {}
};
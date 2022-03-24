#include "session_element.hpp"

#include "../libs/spdlog/fmt/fmt.h"
#include "sql/sql_handle.hpp"

std::string session_element::to_sql_string(const sql_handle& db, const session_element& elem)
{
	return fmt::format("('{}', {}, TO_TIMESTAMP({}), TO_TIMESTAMP({}))",
		db.escape(elem.session_id),
		elem.account_id,
		std::chrono::duration_cast<std::chrono::seconds>(elem.session_creation_time.time_since_epoch()).count(),
		std::chrono::duration_cast<std::chrono::seconds>(elem.last_request_time.time_since_epoch()).count());
}
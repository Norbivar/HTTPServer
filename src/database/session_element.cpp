#include "session_element.hpp"

#include "../libs/format.hpp"
#include "sql/sql_handle.hpp"

std::string session_element::to_sql_string(sql_handle& db, const session_element& elem)
{
	return format_string("('%1%', %2%, FROM_UNIXTIME(%3%))",
		db.escape(elem.session_id),
		elem.account_id,
		elem.session_creation_time.time_since_epoch().count());
}
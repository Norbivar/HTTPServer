#include "sql_handle.hpp"

#include "sql_manager.hpp"

sql_handle::sql_handle(sql_manager& man, std::shared_ptr<pqxx::connection> from) :
	m_connection{ from },
	manager{ man }
{ }

std::string sql_handle::escape(const std::string& what) const
{
	return m_connection->esc(what);
}

sql_work<pqxx::work> sql_handle::start()
{
	return sql_work<pqxx::work>{ *m_connection };
}

sql_work<pqxx::read_transaction> sql_handle::start() const
{
	return sql_work<pqxx::read_transaction>{ *m_connection };
}

sql_handle::~sql_handle()
{
	if (m_connection.use_count() == 1)
		manager.add_handle(m_connection);
}
#pragma once

#include <memory>
#include <pqxx/transaction>

class sql_manager;
namespace pqxx
{ 
	class connection;
}

template<typename T>
class sql_work
{
	friend class sql_handle;

	sql_work(pqxx::connection& conn) :
		_work{ conn }
	{ }

public:
	~sql_work()
	{
		if constexpr (std::is_same_v<T, pqxx::work>)
			_work.commit();
	}

	operator T& () { return _work; }
	T* operator->() { return &_work; }

private:
	T _work;
};

class sql_handle
{
public:
	sql_work<pqxx::work> start();
	sql_work<pqxx::read_transaction> start() const;

	sql_handle(sql_manager& man, std::shared_ptr<pqxx::connection> from);
	~sql_handle();

	std::string escape(const std::string& what) const;
private:
	std::shared_ptr<pqxx::connection> m_connection;
	sql_manager& manager;
};

#pragma once

class database
{
public:

  using connection = pqxx::connection;
  using transaction = pqxx::work;
  using result = pqxx::result;

  inline database(const char* connString);
  inline void prepareSQL(const char* name, const char* sql);
  inline transaction startTransaction();
  inline std::string dbVersion(transaction& txn);

private:

  connection m_conn;

};

inline database::database(const char* connString) : m_conn(connString)
{
}

inline void database::prepareSQL(const char* name, const char* sql)
{
  m_conn.prepare(name, sql);
}

inline database::transaction database::startTransaction()
{
  return transaction(m_conn);
}

inline std::string database::dbVersion(transaction& txn)
{
  result r = txn.exec("SELECT version();");
  return r[0][0].as<std::string>();
}

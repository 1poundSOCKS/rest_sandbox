#pragma once

inline std::optional<std::time_t> convertTimestamp(const char* timestampString)
{
    std::tm tm = {};
    std::istringstream ss(timestampString);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.fail() ? std::optional<std::time_t>() : std::mktime(&tm);
}

inline std::string time_t_to_string(std::time_t t)
{
    std::tm* tm_ptr = std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(tm_ptr, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

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

#pragma once

class database
{
public:

  using connection = pqxx::connection;
  using transaction = pqxx::work;
  using result = pqxx::result;

  struct jobs_record
  {
    std::string transactionId;
    int id;
    std::string name;
  };

  inline database(const char* connString);
  inline transaction startTransaction();
  inline std::string dbVersion(transaction& txn);
  inline int getMaxJobId(transaction& txn);
  inline void insert(transaction& txn, const jobs_record& record);

private:

  inline void initialize();

  connection m_conn;

};

inline database::database(const char* connString) : m_conn(connString)
{
  initialize();
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

inline int database::getMaxJobId(transaction& txn)
{
  int maxId = -1;
  database::result r = txn.exec_prepared("GET_MAX_JOB_ID");
  for( auto row : r )
  {
    auto id = row["id"];
    maxId = id.is_null() ? -1 : id.as<int>();
  }
  return maxId;
}

inline void database::insert(transaction& txn, const jobs_record& record)
{
    txn.exec_prepared("INSERT_JOB", record.transactionId, record.id, record.name);
}

inline void database::initialize()
{
  m_conn.prepare("GET_MAX_JOB_ID", "SELECT MAX(id) as id FROM jobs");
  m_conn.prepare("INSERT_JOB", "INSERT INTO jobs(transaction_id, id, name) VALUES ($1, $2, $3)");
}

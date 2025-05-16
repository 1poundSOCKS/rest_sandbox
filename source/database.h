#pragma once

struct database
{
  database(const char* connString) : conn(connString)
  {
    initialize();
  }

  using connection = pqxx::connection;
  using transaction = pqxx::work;
  using result = pqxx::result;

  connection conn;

  inline std::string dbVersion(transaction& txn)
  {
    result r = txn.exec("SELECT version();");
    return r[0][0].as<std::string>();
  }

  struct jobs_record
  {
    std::string transactionId;
    int id;
    std::string name;
  };

  inline transaction getTransaction()
  {
    return transaction(conn);
  }

  inline void initialize()
  {
    conn.prepare("GET_MAX_JOB_ID", "SELECT MAX(id) as id FROM jobs");
    conn.prepare("INSERT_JOB", "INSERT INTO jobs(transaction_id, id, name) VALUES ($1, $2, $3)");
  }

  inline int getMaxJobId(transaction& txn)
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
  
  inline void insert(transaction& txn, const jobs_record& record)
  {
      txn.exec_prepared("INSERT_JOB", record.transactionId, record.id, record.name);
  }
};

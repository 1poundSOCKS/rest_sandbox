#pragma once

namespace database
{
  using transaction = pqxx::work;
  using connection = pqxx::connection;
  using result = pqxx::result;

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
  
  inline void insert(transaction& txn, const jobs_record& record)
  {
      txn.exec_params("INSERT INTO jobs(transaction_id, id, name) VALUES ($1, $2, $3)", record.transactionId, record.id, record.name);
  }
};

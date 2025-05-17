#pragma once

#include "database.h"

static constexpr char* preparedGetMaxJobId = "GET_MAX_JOB_ID";

inline void prepareGetMaxJobId(database& db)
{
  db.prepareSQL(preparedGetMaxJobId, "SELECT MAX(id) as id FROM jobs");
}

inline int getMaxJobId(database::transaction& txn)
{
  int maxId = -1;
  database::result r = txn.exec_prepared(preparedGetMaxJobId);
  for( auto row : r )
  {
    auto id = row["id"];
    maxId = id.is_null() ? -1 : id.as<int>();
  }
  return maxId;
}

struct jobs_record
{
  std::string transactionId;
  int id;
  std::string name;
};

static constexpr char* preparedInsertJob = "INSERT_JOB";

inline void prepareInsertJob(database& db)
{
  db.prepareSQL(preparedInsertJob, "INSERT INTO jobs(transaction_id, id, name) VALUES ($1, $2, $3)");
}

inline void insert(database::transaction& txn, const jobs_record& record)
{
    txn.exec_prepared(preparedInsertJob, record.transactionId, record.id, record.name);
}

inline void prepareSQL(database& db)
{
  prepareGetMaxJobId(db);
  prepareInsertJob(db);
}

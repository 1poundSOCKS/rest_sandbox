#pragma once

#include "database.h"

static constexpr char* preparedGetMaxJobId = "GET_MAX_JOB_ID";

inline void prepareGetMaxJobId(database& db)
{
  db.prepareSQL(preparedGetMaxJobId, "SELECT MAX(job_id) as max_job_id FROM jobs");
}

inline int getMaxJobId(database::transaction& txn)
{
  int maxId = -1;
  database::result r = txn.exec_prepared(preparedGetMaxJobId);
  for( auto row : r )
  {
    auto id = row["max_job_id"];
    maxId = id.is_null() ? -1 : id.as<int>();
  }
  return maxId;
}

struct jobs_record
{
  std::time_t transactionTimestamp;
  std::string transactionId;
  int64_t jobId;
  std::string jobName;
};

static constexpr char* preparedInsertJob = "INSERT_JOB";

inline void prepareInsertJob(database& db)
{
  db.prepareSQL(preparedInsertJob, "INSERT INTO jobs(transaction_timestamp, transaction_id, job_id, job_name) VALUES ($1, $2, $3, $4)");
}

inline void insert(database::transaction& txn, const jobs_record& record)
{
    txn.exec_prepared(preparedInsertJob, record.transactionTimestamp, record.transactionId, record.jobId, record.jobName);
}

inline void prepareSQL(database& db)
{
  prepareGetMaxJobId(db);
  prepareInsertJob(db);
}

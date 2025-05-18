#pragma once

#include "database.h"

std::string toString(std::time_t timestamp)
{
  std::tm* localTime = std::localtime(&timestamp);
  std::ostringstream oss;
  oss << std::put_time(localTime, "%Y-%m-%d %H:%M:%S");
  return oss.str();
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
    txn.exec_prepared(preparedInsertJob, toString(record.transactionTimestamp), record.transactionId, record.jobId, record.jobName);
}

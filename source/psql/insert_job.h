#pragma once

#include "database.h"

namespace psql
{

  inline std::string toString(std::time_t timestamp)
  {
    std::tm* localTime = std::localtime(&timestamp);
    std::ostringstream oss;
    oss << std::put_time(localTime, "%Y-%m-%d %H:%M:%S");
    return oss.str();
  }

  struct insert_job_in
  {
    std::time_t transactionTimestamp;
    std::string transactionId;
    int64_t jobId;
    std::string jobName;
  };

  static constexpr const char* preparedInsertJob = "INSERT_JOB";

  inline void prepareInsertJob(database& db)
  {
    db.prepareSQL(preparedInsertJob, "INSERT INTO jobs(transaction_timestamp, transaction_id, job_id, job_name) VALUES ($1, $2, $3, $4)");
  }

  inline void insertJob(database::transaction& txn, const insert_job_in& record)
  {
      txn.exec_prepared(preparedInsertJob, pqxx::params(
        std::string(toString(record.transactionTimestamp)), 
        std::string(record.transactionId), 
        static_cast<int64_t>(record.jobId), 
        std::string(record.jobName)));
  }

};

#pragma once

#include "database.h"

namespace psql
{

  struct insert_job_in
  {
    int64_t jobId;
    std::string jobName;
  };

  static constexpr const char* preparedInsertJob = "INSERT_JOB";

  inline void prepareInsertJob(std::shared_ptr<database> db)
  {
    db->prepareSQL(preparedInsertJob, "INSERT INTO jobs(transaction_timestamp, transaction_id, job_id, job_name) VALUES ($1, $2, $3, $4)");
  }

  inline void insertJob(database::transaction& txn, 
    std::time_t transactionTimestamp,
    const char* transactionId,
    const insert_job_in& record)
  {
      txn.exec_prepared(preparedInsertJob, pqxx::params(
        time_t_to_string(transactionTimestamp), 
        transactionId, 
        static_cast<int64_t>(record.jobId), 
        record.jobName));
  }

};

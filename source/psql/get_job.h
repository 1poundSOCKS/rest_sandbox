#pragma once

#include "database.h"

namespace psql
{

  static const char* preparedGetJob = "GET_JOB";

  inline void prepareGetJob(std::shared_ptr<database> db)
  {
    db->prepareSQL(preparedGetJob, 
      "SELECT transaction_id, transaction_time, job_id, job_name "
      "FROM jobs "
      "where job_id = $1 "
      "ORDER BY transaction_time, transaction_id DESC "
      "LIMIT 1");
  }

  struct get_job_out
  {
    std::string transactionId;
    std::time_t transactionTime;
    std::string jobName;
  };

  inline std::optional<get_job_out> getJob(database::transaction& txn, int64_t jobId)
  {
    database::result r = txn.exec_prepared(preparedGetJob, static_cast<int64_t>(jobId));

    if( std::begin(r) != std::end(r) )
    {
      auto&& row = r.front();
      auto transactionId = row["transaction_id"];
      auto transactionTime = row["transaction_time"];
      auto jobName = row["job_name"];

      auto convertedTime = convertTime(transactionTime.as<std::string>().c_str());

      return convertedTime.has_value() ? get_job_out{
        transactionId.as<std::string>(),
        convertedTime.value(),
        jobName.as<std::string>()
      } : std::optional<get_job_out>();
    }
    else
    {
      return std::nullopt;
    }
  }

};

#pragma once

#include "database.h"

static constexpr char* preparedGetJob = "GET_JOB";

inline void prepareGetJob(database& db)
{
  db.prepareSQL(preparedGetJob, "SELECT job_id, job_name FROM jobs where job_id = $1 ORDER BY transaction_timestamp DESC");
}

struct get_job_out
{
  std::string jobName;
};

inline std::optional<get_job_out> getJob(database::transaction& txn, int64_t jobId)
{
  database::result r = txn.exec_prepared(preparedGetJob, jobId);

  if( std::begin(r) != std::end(r) )
  {
    auto&& row = r.front();
    auto jobName = row["job_name"];
    return get_job_out { jobName.as<std::string>() };
  }
  else
  {
    return std::nullopt;
  }
}

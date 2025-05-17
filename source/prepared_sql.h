#pragma once

#include "database.h"

std::string toString(std::time_t timestamp)
{
  std::tm* localTime = std::localtime(&timestamp);
  std::ostringstream oss;
  oss << std::put_time(localTime, "%Y-%m-%d %H:%M:%S");
  return oss.str();
}

// GetMaxJobId
//

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

// InsertJob
//

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

// GetJob
//

static constexpr char* preparedGetJob = "GET_JOB";

inline void prepareGetJob(database& db)
{
  db.prepareSQL(preparedGetJob, "SELECT job_id, job_name FROM jobs where job_id = $1 ORDER BY transaction_timestamp DESC");
}

struct get_job_data
{
  std::string jobName;
};

inline std::optional<get_job_data> getJob(database::transaction& txn, int64_t jobId)
{
  database::result r = txn.exec_prepared(preparedGetJob, jobId);

  if( std::begin(r) != std::end(r) )
  {
    auto&& row = r.front();
    auto jobName = row["job_name"];
    return get_job_data { jobName.as<std::string>() };
  }
  else
  {
    return std::nullopt;
  }
}

// prepare all SQL
//

inline void prepareSQL(database& db)
{
  prepareGetMaxJobId(db);
  prepareInsertJob(db);
  prepareGetJob(db);
}

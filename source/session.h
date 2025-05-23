#pragma once

#include "database.h"
#include "psql/psql.h"

struct book_job_request_data
{
  std::optional<int64_t> jobId;
  std::string jobName;
};

struct book_job_response_data
{
  int64_t code;
  int64_t jobId;
};

struct get_job_request_data
{
  int64_t jobId;
};

struct get_job_response_data
{
  int64_t code;
  std::time_t transactionTime;
  std::string transactionId;
  std::optional<int64_t> jobId;
  std::optional<std::string> jobName;
};

class session
{
public:
  session(const char* dbConnection);
  void initialize();
  std::string dbVersion();
  book_job_response_data run(const book_job_request_data& requestData);
  std::optional<get_job_response_data> run(const get_job_request_data& requestData);

private:

  std::string m_dbConnection;
  database m_db;
  int64_t m_maxJobId;
};

inline session::session(const char* dbConnection) : m_dbConnection(dbConnection), m_maxJobId(-1), m_db(dbConnection)
{
  psql::prepareSQL(m_db);
  database::transaction txn = m_db.startTransaction();
  m_maxJobId = psql::getMaxJobId(txn);
  txn.commit();
}

inline std::string session::dbVersion()
{
  std::string version = "";
  database::transaction txn = m_db.startTransaction();
  version = m_db.dbVersion(txn);
  txn.commit();
  return version;
}

inline book_job_response_data session::run(const book_job_request_data& requestData)
{
  std::time_t now = std::time(nullptr);
  boost::uuids::uuid uuid = boost::uuids::random_generator()();
  std::string uuidStr = boost::lexical_cast<std::string>(uuid);
  auto jobId = requestData.jobId.has_value() ? requestData.jobId.value() : ++m_maxJobId;
  psql::insert_job_in in { jobId, requestData.jobName };
  
  database::transaction txn = m_db.startTransaction();
  psql::insertJob(txn, now, uuidStr.c_str(), in);
  txn.commit();
  return book_job_response_data { 0, jobId };
}

inline std::optional<get_job_response_data> session::run(const get_job_request_data& requestData)
{
  database::transaction txn = m_db.startTransaction();
  auto outputData = psql::getJob(txn, requestData.jobId);
  txn.commit();
  
  return outputData.has_value() ? get_job_response_data {
      0, 
      outputData->transactionTime, 
      outputData->transactionId, 
      requestData.jobId, 
      outputData->jobName
    } : std::optional<get_job_response_data>();
}

#pragma once

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
  std::optional<int64_t> jobId;
  std::optional<std::string> jobName;
};

using command_data = std::variant<book_job_request_data, get_job_request_data>;
using response_data = std::variant<book_job_response_data, get_job_response_data>;

class session
{
public:
  session(const char* dbConnection);
  void initialize();
  std::string dbVersion();
  response_data run(const command_data& commandData);

private:

  response_data run(const book_job_request_data& requestData);
  response_data run(const get_job_request_data& requestData);

  std::string m_dbConnection;
  database m_db;
  int64_t m_maxJobId;
};

inline session::session(const char* dbConnection) : m_dbConnection(dbConnection), m_maxJobId(-1), m_db(dbConnection)
{
  prepareSQL(m_db);
  database::transaction txn = m_db.startTransaction();
  m_maxJobId = getMaxJobId(txn);
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

inline response_data session::run(const command_data& commandData)
{
  return std::visit([this](auto&& requestData)
  {
    return run(requestData);
  }, commandData);
}

inline response_data session::run(const book_job_request_data& requestData)
{
  std::time_t now = std::time(nullptr);
  boost::uuids::uuid uuid = boost::uuids::random_generator()();
  std::string uuidStr = boost::lexical_cast<std::string>(uuid);
  auto jobId = requestData.jobId.has_value() ? requestData.jobId.value() : ++m_maxJobId;
  jobs_record record { now, uuidStr, jobId, requestData.jobName };
  
  database::transaction txn = m_db.startTransaction();
  insert(txn, record);
  txn.commit();
  return book_job_response_data { 0, jobId };
}

inline response_data session::run(const get_job_request_data& requestData)
{
  database::transaction txn = m_db.startTransaction();
  auto outputData = getJob(txn, requestData.jobId);
  txn.commit();
  return outputData.has_value() ? get_job_response_data { 0, requestData.jobId, outputData->jobName } : get_job_response_data { 1, std::nullopt, std::nullopt };
}

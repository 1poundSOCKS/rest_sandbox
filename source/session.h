#pragma once

#include "prepared_sql.h"

struct book_job_request_data
{
  std::string jobName;
};

struct book_job_response_data
{
  int64_t jobId;
};

using command_data = std::variant<book_job_request_data>;
using response_data = std::variant<book_job_response_data>;

class session
{
public:
  session(const char* dbConnection);
  void initialize();
  std::string dbVersion();
  response_data run(const command_data& commandData);

private:

  book_job_response_data run(const book_job_request_data& commandData);

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
  return std::visit([this](auto&& commandData)
  {
    return run(commandData);
  }, commandData);
}

inline book_job_response_data session::run(const book_job_request_data& request)
{
  std::time_t now = std::time(nullptr);
  boost::uuids::uuid uuid = boost::uuids::random_generator()();
  std::string uuidStr = boost::lexical_cast<std::string>(uuid);
  auto jobId = ++m_maxJobId;
  jobs_record record { now, uuidStr, jobId, request.jobName };
  
  database::transaction txn = m_db.startTransaction();
  insert(txn, record);
  txn.commit();
  return { jobId };
}

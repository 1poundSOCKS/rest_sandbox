#pragma once

#include "database.h"
#include "psql/psql.h"
#include "commands/command.h"
#include "job.h"

class session
{
public:
  session(const char* dbConnection);
  void initialize();
  std::string dbVersion();
  std::optional<book_job_response> bookJob(book_job_request requestData);
  std::optional<get_job_response> getJob(get_job_request requestData);

private:

  std::string m_dbConnection;
  std::shared_ptr<database> m_db;
  int64_t m_maxJobId;
  std::map<int64_t, job> m_jobs;
};

inline session::session(const char* dbConnection) : 
  m_dbConnection(dbConnection), m_maxJobId(-1), m_db(std::make_shared<database>(dbConnection))
{
  psql::prepareSQL(m_db);
  database::transaction txn = m_db->startTransaction();
  m_maxJobId = psql::getMaxJobId(txn);
  txn.commit();
}

inline std::string session::dbVersion()
{
  std::string version = "";
  database::transaction txn = m_db->startTransaction();
  version = m_db->dbVersion(txn);
  txn.commit();
  return version;
}

inline std::optional<book_job_response> session::bookJob(book_job_request requestData)
{
  auto jobId = requestData.jobId.has_value() ? requestData.jobId.value() : ++m_maxJobId;
  requestData.jobId = jobId;
  transaction trx;
  auto response = ::bookJob(m_db, trx, requestData);
  m_jobs[jobId] = { trx, jobId, requestData.jobName, requestData.duration };
  return response;
}

inline std::optional<get_job_response> session::getJob(get_job_request requestData)
{
  auto foundJob = m_jobs.find(requestData.jobId);
  
  if( foundJob == std::end(m_jobs) )
  {
    auto response = ::getJob(m_db, requestData);

    if( response.has_value() && response->code == 0 && response->jobData.has_value() )
    {
      auto&& job = response->jobData.value();
      auto insertedJob = m_jobs.insert({job.id, { job.trx, job.id, job.name, job.duration}});
      return response;
    }
  }
  else
  {
    return get_job_response { 0U, foundJob->second };
  }
}

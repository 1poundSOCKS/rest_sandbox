#pragma once

#include "tables.h"

struct book_job_data
{
  std::string name;
};

using command_data = std::variant<book_job_data>;

class session
{
public:
  session(const char* dbConnection);
  void initialize();
  std::string dbVersion();
  void run(const command_data& commandData);

private:

  static constexpr char* m_getMaxJobId = "GET_MAX_JOB_ID";
  static constexpr char* m_insertJob = "INSERT_JOB";

  void run(const book_job_data& commandData);
  int getMaxJobId();

  std::string m_dbConnection;
  database m_db;
  int m_maxJobId;
};

inline session::session(const char* dbConnection) : m_dbConnection(dbConnection), m_maxJobId(-1), m_db(dbConnection)
{
  m_db.prepareSQL(m_getMaxJobId, "SELECT MAX(id) as id FROM jobs");
  m_db.prepareSQL(m_insertJob, "INSERT INTO jobs(transaction_id, id, name) VALUES ($1, $2, $3)");
  m_maxJobId = getMaxJobId();
}

inline std::string session::dbVersion()
{
  std::string version = "";

  database::transaction txn = m_db.startTransaction();
  version = m_db.dbVersion(txn);
  txn.commit();

  return version;
}

inline int session::getMaxJobId()
{
  int maxId = -1;
  database::transaction txn = m_db.startTransaction();
  maxId = m_db.getMaxJobId(txn, m_getMaxJobId);
  txn.commit();
  return maxId;
}

inline void session::run(const command_data& commandData)
{
  std::visit([this](auto&& commandData)
  {
    run(commandData);
  }, commandData);
}

inline void session::run(const book_job_data& commandData)
{
  boost::uuids::uuid uuid = boost::uuids::random_generator()();
  std::string uuidStr = boost::lexical_cast<std::string>(uuid);
  jobs_record record { uuidStr, ++m_maxJobId, commandData.name };
  
  database::transaction txn = m_db.startTransaction();
  insert(txn, m_insertJob, record);
  txn.commit();
}

#pragma once

#include "prepared_sql.h"

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

  void run(const book_job_data& commandData);

  std::string m_dbConnection;
  database m_db;
  int m_maxJobId;
};

inline session::session(const char* dbConnection) : m_dbConnection(dbConnection), m_maxJobId(-1), m_db(dbConnection)
{
  prepareGetMaxJobId(m_db);
  prepareJobsInsert(m_db);
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
  insert(txn, record);
  txn.commit();
}

#pragma once

#include "database.h"

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
  int getMaxJobId();

  std::string m_dbConnection;
  database m_db;
  int m_maxJobId;
};

inline session::session(const char* dbConnection) : m_dbConnection(dbConnection), m_maxJobId(-1), m_db(dbConnection)
{
}

inline void session::initialize()
{
  m_db.initialize();
  m_maxJobId = getMaxJobId();
}

inline std::string session::dbVersion()
{
  std::string version = "";

  database::transaction txn = m_db.getTransaction();
  version = m_db.dbVersion(txn);
  txn.commit();

  return version;
}

inline int session::getMaxJobId()
{
  int maxId = -1;
  database::transaction txn = m_db.getTransaction();
  maxId = m_db.getMaxJobId(txn);
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
  database::jobs_record record { uuidStr, ++m_maxJobId, commandData.name };
  
  database::transaction txn = m_db.getTransaction();
  m_db.insert(txn, record);
  txn.commit();
}

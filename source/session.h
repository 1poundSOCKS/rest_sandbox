#pragma once

#include "database.h"

struct unknown_data
{
};

struct book_job_data
{
  std::string name;
};

using command_data = std::variant<unknown_data, book_job_data>;

class session
{
public:
  session(const char* dbConnection);
  void initialize();
  std::string dbVersion() const;
  void run(const command_data& commandData);

private:

  void run(const book_job_data& commandData);
  int getMaxJobId() const;

  std::string m_dbConnection;
  int m_maxJobId;
};

inline session::session(const char* dbConnection) : m_dbConnection(dbConnection), m_maxJobId(-1)
{
}

inline void session::initialize()
{
  m_maxJobId = getMaxJobId();
}

inline std::string session::dbVersion() const
{
  std::string version = "";

  database::connection conn(m_dbConnection);
  
  if( conn.is_open() )
  {
    database::transaction txn(conn);
    version = database::dbVersion(txn);
    txn.commit();
  }

  return version;
}

inline int session::getMaxJobId() const
{
  int maxId = -1;

  database::connection conn(m_dbConnection);
  
  if( conn.is_open() )
  {
    database::transaction txn(conn);

    database::result r = txn.exec_params("SELECT MAX(id) as id FROM jobs");
    for( auto row : r )
    {
      auto id = row["id"];
      maxId = id.is_null() ? -1 : id.as<int>();
    }
    txn.commit();
  }

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
  database::connection conn(m_dbConnection);

  if( conn.is_open() )
  {
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    std::string uuidStr = boost::lexical_cast<std::string>(uuid);
    database::jobs_record record { uuidStr, ++m_maxJobId, commandData.name };
    
    database::transaction txn(conn);
    database::insert(txn, record);
    txn.commit();
  }
}

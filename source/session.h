#pragma once

struct job_data
{
  int id;
  std::string name;
};

class session
{
public:
  session(const char* dbConnection);
  std::string dbVersion() const;
  void run(const std::variant<job_data>& commandData);

private:

  void run(const job_data& commandData);

  std::string m_dbConnection;
};

inline session::session(const char* dbConnection) : m_dbConnection(dbConnection)
{
}

inline std::string session::dbVersion() const
{
  std::string dbVersion = "***unknown***";

  pqxx::connection conn(m_dbConnection);

  if( conn.is_open() )
  {
    pqxx::work txn(conn);
    pqxx::result r = txn.exec("SELECT version();");
    dbVersion = r[0][0].as<std::string>();
    txn.commit();
  }

  return dbVersion;
}

inline void session::run(const std::variant<job_data>& commandData)
{
  pqxx::connection conn(m_dbConnection);

  std::visit([this](const job_data& commandData)
  {
    run(commandData);
  }, commandData);
}

inline void session::run(const job_data& commandData)
{
  pqxx::connection conn(m_dbConnection);

  if( conn.is_open() )
  {
    pqxx::work txn(conn);
    txn.exec_params("INSERT INTO jobs(id,name) VALUES ($1,$2)", commandData.id, commandData.name);
    txn.commit();
  }
}

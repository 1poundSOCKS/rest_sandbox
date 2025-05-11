#pragma once

class session
{
public:
  session(const char* dbConnection);
  std::string dbVersion() const;
  void createJob(int id, const char* name);

private:
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

inline void session::createJob(int id, const char *name)
{
    pqxx::connection conn(m_dbConnection);

    if( conn.is_open() )
    {
      pqxx::work txn(conn);
      txn.exec_params("INSERT INTO jobs(id,name) VALUES ($1,$2)", id, name);
      txn.commit();
    }
}

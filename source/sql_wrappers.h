#pragma once

#include <sql.h>
#include <sqlext.h>

inline void show_error(unsigned int handleType, const SQLHANDLE& handle)
{
  SQLCHAR SQLState[1024];
  SQLCHAR message[1024];

  if (SQLGetDiagRec(handleType, handle, 1, SQLState, NULL, message, 1024, NULL) == SQL_SUCCESS)
  {
      std::cout << "ODBC Error: " << message << "\nSQL State: " << SQLState << std::endl;
  }
}

class sql_handle
{

  public:
    sql_handle();
    ~sql_handle();
    operator SQLHANDLE() const;
    bool setEnvAttr() const;

  private:
    SQLHANDLE m_env;

};

inline sql_handle::sql_handle() : m_env(nullptr)
{
  SQLRETURN ret = ::SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_env);

  if( !SQL_SUCCEEDED(ret) )
  {
    m_env = nullptr;
  }
}

inline sql_handle::~sql_handle()
{
  if( m_env )
  {
    SQLFreeHandle(SQL_HANDLE_ENV, m_env);
    m_env = nullptr;
  }
}

inline sql_handle::operator SQLHANDLE() const
{
  return m_env;
}

inline bool sql_handle::setEnvAttr() const
{
  SQLRETURN ret = ::SQLSetEnvAttr(m_env, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
  return SQL_SUCCEEDED(ret);
}

class dbc_handle
{
  public:
    dbc_handle(SQLHANDLE env);
    ~dbc_handle();
    operator SQLHDBC() const;
    void connect(SQLCHAR* connStr);
    bool isConnected() const;

  private:
    SQLHDBC m_dbc;
    bool m_connected;
};

inline dbc_handle::dbc_handle(SQLHANDLE env) : m_dbc(nullptr), m_connected(false)
{
  SQLRETURN ret = ::SQLAllocHandle(SQL_HANDLE_DBC, env, &m_dbc);

  if( !SQL_SUCCEEDED(ret) )
  {
    m_dbc = nullptr;
  }
}

inline dbc_handle::~dbc_handle()
{
  if( m_connected )
  {
    ::SQLDisconnect(m_dbc);
    m_connected = false;
  }

  if( m_dbc )
  {
    ::SQLFreeHandle(SQL_HANDLE_DBC, m_dbc);
    m_dbc = nullptr;
  }
}

inline dbc_handle::operator SQLHDBC() const
{
  return m_dbc;
}

inline void dbc_handle::connect(SQLCHAR* connStr)
{
  if( !m_connected )
  {
    SQLRETURN ret = ::SQLDriverConnect(m_dbc, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);
    m_connected = SQL_SUCCEEDED(ret);  
  }
}

inline bool dbc_handle::isConnected() const
{
  return m_connected;
}

class sql_statement
{
public:
  sql_statement(SQLHANDLE dbc);
  ~sql_statement();
  operator SQLHSTMT() const;
  bool prepare(SQLCHAR* StatementText, SQLINTEGER TextLength) const;
  bool bindParameter(SQLUSMALLINT ParameterNumber, SQLSMALLINT InputOutputType, SQLSMALLINT ValueType, SQLSMALLINT ParameterType, 
    SQLULEN ColumnSize, SQLSMALLINT DecimalDigits, SQLPOINTER ParameterValuePtr, SQLLEN BufferLength, SQLLEN* StrLen_or_IndPtr) const;
  bool execute() const;

private:
  SQLHSTMT m_stmt;
};

inline sql_statement::sql_statement(SQLHANDLE dbc) : m_stmt(nullptr)
{
  SQLRETURN ret = ::SQLAllocHandle(SQL_HANDLE_STMT, dbc, &m_stmt);

  if( !SQL_SUCCEEDED(ret) )
  {
    m_stmt = nullptr;
  }
}

inline sql_statement::~sql_statement()
{
  if( m_stmt )
  {
    ::SQLFreeHandle(SQL_HANDLE_STMT, m_stmt);
    m_stmt = nullptr;
  }
}

inline sql_statement::operator SQLHSTMT() const
{
  return m_stmt;
}

inline bool sql_statement::prepare(SQLCHAR* StatementText, SQLINTEGER TextLength) const
{
  SQLRETURN ret = ::SQLPrepare(m_stmt, StatementText, TextLength);
  return SQL_SUCCEEDED(ret);
}

inline bool sql_statement::execute() const
{
  SQLRETURN ret = ::SQLExecute(m_stmt);
  return SQL_SUCCEEDED(ret);
}

inline bool sql_statement::bindParameter(SQLUSMALLINT ParameterNumber, SQLSMALLINT InputOutputType, SQLSMALLINT ValueType, SQLSMALLINT ParameterType, 
  SQLULEN ColumnSize, SQLSMALLINT DecimalDigits, SQLPOINTER ParameterValuePtr, SQLLEN BufferLength, SQLLEN* StrLen_or_IndPtr) const
{
  SQLRETURN ret = ::SQLBindParameter(m_stmt,ParameterNumber,InputOutputType,ValueType,ParameterType,ColumnSize,DecimalDigits,ParameterValuePtr,BufferLength,StrLen_or_IndPtr);
  return SQL_SUCCEEDED(ret);
}

namespace sql_statements
{
  class insert_job
  {
    public:
      struct data
      {
        data();
        int jobId;
      };

      data data;

      insert_job(SQLHANDLE dbc);
      operator SQLHSTMT() const;
      bool execute() const;


    private:
      sql_statement m_statement;
  };

  inline insert_job::data::data() : jobId(-1)
  {
  }

  inline insert_job::insert_job(SQLHANDLE dbc) : m_statement(dbc)
  {
    SQLCHAR* query = (SQLCHAR*)"INSERT INTO jobs (id) VALUES (?)";
    m_statement.prepare(query, SQL_NTS);
    m_statement.bindParameter(1,SQL_PARAM_INPUT,SQL_C_LONG,SQL_INTEGER,0,SQL_INTEGER,&(data.jobId),0,NULL);
  }

  inline insert_job::operator SQLHSTMT() const
  {
    return m_statement;
  }

  inline bool insert_job::execute() const
  {
    return m_statement.execute();
  }

};

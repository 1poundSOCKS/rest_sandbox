#pragma once

#include "database.h"

struct jobs_record
{
  std::string transactionId;
  int id;
  std::string name;
};

inline void insert(database::transaction& txn, const char* statement, const jobs_record& record)
{
    txn.exec_prepared(statement, record.transactionId, record.id, record.name);
}

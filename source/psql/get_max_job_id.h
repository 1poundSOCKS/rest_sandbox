#pragma once

#include "database.h"

namespace psql
{

  static constexpr char* preparedGetMaxJobId = "GET_MAX_JOB_ID";

  inline void prepareGetMaxJobId(database& db)
  {
    db.prepareSQL(preparedGetMaxJobId, "SELECT MAX(job_id) as max_job_id FROM jobs");
  }

  inline int64_t getMaxJobId(database::transaction& txn)
  {
    int maxId = -1;
    database::result r = txn.exec_prepared(preparedGetMaxJobId);
    for( auto row : r )
    {
      auto id = row["max_job_id"];
      maxId = id.is_null() ? -1 : id.as<int>();
    }
    return maxId;
  }

};

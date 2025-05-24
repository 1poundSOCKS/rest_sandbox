#pragma once

#include "get_max_job_id.h"
#include "insert_job.h"
#include "get_job.h"

namespace psql
{
  inline void prepareSQL(std::shared_ptr<database> db)
  {
    prepareGetMaxJobId(db);
    prepareInsertJob(db);
    prepareGetJob(db);
  }
};

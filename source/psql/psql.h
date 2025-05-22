#pragma once

namespace psql
{
  #include "get_max_job_id.h"
  #include "insert_job.h"
  #include "get_job.h"

  inline void prepareSQL(database& db)
  {
    prepareGetMaxJobId(db);
    prepareInsertJob(db);
    prepareGetJob(db);
  }
};

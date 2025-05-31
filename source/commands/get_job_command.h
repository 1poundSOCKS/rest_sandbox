#pragma once

#include <database.h>
#include "job.h"

struct get_job_request
{
  int64_t jobId;
};

struct get_job_response
{
  int64_t code;
  std::optional<job> jobData;
};

inline void operator >>(nlohmann::json requestJson, get_job_request& request)
{
    request.jobId = requestJson["body"]["job_id"];
}

inline void operator <<(nlohmann::json& responseJson, std::optional<get_job_response> response)
{
  if( response.has_value() )
  {
    responseJson["header"]["code"] = response->code;

    if( response->jobData.has_value() )
    {
      auto&& job = response->jobData.value();
      responseJson["body"]["transaction_timestamp"] = time_t_to_string(job.trx.time);
      responseJson["body"]["transaction_id"] = job.trx.id;
      responseJson["body"]["job_id"] = job.id;
      responseJson["body"]["job_name"] = job.name;
    }
  }
}

inline std::optional<get_job_response> getJob(std::shared_ptr<database> db, get_job_request requestData)
{
  std::optional<get_job_response> responseData;

  database::transaction txn = db->startTransaction();
  auto outputData = psql::getJob(txn, requestData.jobId);
  txn.commit();

  if( outputData.has_value() )
  {
    responseData = get_job_response();
    responseData->code = 0;

    responseData->jobData = {
      {
        outputData->transactionId,
        outputData->transactionTime 
      }, 
      requestData.jobId, 
      outputData->jobName
    };
  }
  else
  {
    responseData = get_job_response();
    responseData->code = 1;
  }

  return responseData;
}

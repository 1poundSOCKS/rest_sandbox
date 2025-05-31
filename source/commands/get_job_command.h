#pragma once

#include <database.h>

struct get_job_request
{
  int64_t jobId;
};

struct get_job_response
{
  struct body_type
  {
    std::time_t transactionTime;
    std::string transactionId;
    int64_t jobId;
    std::string jobName;
  };

  int64_t code;
  std::optional<body_type> body;
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

    if( response->body.has_value() )
    {
      auto&& body = response->body.value();
      responseJson["body"]["transaction_timestamp"] = time_t_to_string(body.transactionTime);
      responseJson["body"]["transaction_id"] = body.transactionId;
      responseJson["body"]["job_id"] = body.jobId;
      responseJson["body"]["job_name"] = body.jobName;
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

    responseData->body = get_job_response::body_type
    {
      outputData->transactionTime, 
      outputData->transactionId, 
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

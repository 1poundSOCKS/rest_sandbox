#pragma once

#include <database.h>

namespace get_job_cmd
{
  struct request_data
  {
    int64_t jobId;
  };

  struct response_data
  {
    int64_t code;
    std::time_t transactionTime;
    std::string transactionId;
    std::optional<int64_t> jobId;
    std::optional<std::string> jobName;
  };

  inline request_data formatRequest(nlohmann::json requestJson)
  {
      request_data requestData;
      requestData.jobId = requestJson["job_id"];
      return requestData;
  }

  inline nlohmann::json formatResponse(get_job_cmd::response_data responseData)
  {
    nlohmann::json responseJson;
    responseJson["code"] = responseData.code;
    responseJson["transaction_timestamp"] = time_t_to_string(responseData.transactionTime);
    responseJson["transaction_id"] = responseData.transactionId;
    if( responseData.jobId.has_value() ) responseJson["job_id"] = responseData.jobId.value();
    if( responseData.jobName.has_value() ) responseJson["job_name"] = responseData.jobName.value();
    return responseJson;
  }

  inline std::optional<response_data> run(std::shared_ptr<database> db, request_data requestData)
  {
    database::transaction txn = db->startTransaction();
    auto outputData = psql::getJob(txn, requestData.jobId);
    txn.commit();
    
    return outputData.has_value() ? response_data {
        0, 
        outputData->transactionTime, 
        outputData->transactionId, 
        requestData.jobId, 
        outputData->jobName
      } : std::optional<response_data>();
  }
};

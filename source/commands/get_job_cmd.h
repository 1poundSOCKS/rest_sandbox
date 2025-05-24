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

  inline request_data formatRequest(nlohmann::json requestJson)
  {
      request_data requestData;
      requestData.jobId = requestJson["job_id"];
      return requestData;
  }

  inline nlohmann::json formatResponse(get_job_cmd::response_data responseData)
  {
    nlohmann::json responseJson;
    responseJson["header"]["code"] = responseData.code;
 
    if( responseData.body.has_value() )
    {
      auto&& body = responseData.body.value();
      responseJson["body"]["transaction_timestamp"] = time_t_to_string(body.transactionTime);
      responseJson["body"]["transaction_id"] = body.transactionId;
      responseJson["body"]["job_id"] = body.jobId;
      responseJson["body"]["job_name"] = body.jobName;
    }

    return responseJson;
  }

  inline std::optional<response_data> run(std::shared_ptr<database> db, request_data requestData)
  {
    std::optional<response_data> responseData;

    database::transaction txn = db->startTransaction();
    auto outputData = psql::getJob(txn, requestData.jobId);
    txn.commit();

    if( outputData.has_value() )
    {
      responseData = response_data();
      responseData->code = 0;

      responseData->body = response_data::body_type
      {
        outputData->transactionTime, 
        outputData->transactionId, 
        requestData.jobId, 
        outputData->jobName
      };
    }
    else
    {
      responseData = response_data();
      responseData->code = 1;
    }

    return responseData;
  }
};

#pragma once

#include <database.h>

namespace book_job_cmd
{
  struct request_data
  {
    std::optional<int64_t> jobId;
    std::string jobName;
    unsigned int duration;
  };

  struct response_data
  {
    int64_t code;
    int64_t jobId;
  };

  inline request_data formatRequest(nlohmann::json requestJson)
  {
    request_data requestData;
    if( requestJson.contains("body"))
    {
      auto&& body = requestJson["body"];

      if( body.is_object() )
      {
        requestData.jobId = body.contains("job_id") ? body["job_id"] : std::optional<int64_t>();
        requestData.jobName = body["job_name"];
        requestData.duration = body["duration"];
      }
    }
    return requestData;
  }

  inline nlohmann::json formatResponse(book_job_cmd::response_data responseData)
  {
    nlohmann::json responseJson;
    responseJson["header"]["code"] = responseData.code;
    responseJson["body"]["job_id"] = responseData.jobId;
    return responseJson;
  }

  inline std::optional<response_data> run(std::shared_ptr<database> db, request_data requestData)
  {
    std::time_t now = std::time(nullptr);
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    std::string uuidStr = boost::lexical_cast<std::string>(uuid);
    psql::insert_job_in in { requestData.jobId.value(), requestData.jobName, requestData.duration };
    
    database::transaction txn = db->startTransaction();
    psql::insertJob(txn, now, uuidStr.c_str(), in);
    txn.commit();
    return response_data { 0, requestData.jobId.value() };
  }
};

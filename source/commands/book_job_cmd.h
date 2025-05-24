#pragma once

#include <database.h>

namespace book_job_cmd
{
  struct request_data
  {
    std::optional<int64_t> jobId;
    std::string jobName;
  };

  struct response_data
  {
    int64_t code;
    int64_t jobId;
  };

  inline request_data formatRequest(nlohmann::json requestJson)
  {
    request_data requestData;
    requestData.jobId = requestJson.contains("job_id") ? requestJson["job_id"] : std::optional<int64_t>();
    requestData.jobName = requestJson["job_name"];
    return requestData;
  }

  inline nlohmann::json formatResponse(book_job_cmd::response_data responseData)
  {
    nlohmann::json responseJson;
    responseJson["code"] = responseData.code;
    responseJson["job_id"] = responseData.jobId;
    return responseJson;
  }

  inline std::optional<response_data> run(std::shared_ptr<database> db, request_data requestData)
  {
    std::time_t now = std::time(nullptr);
    boost::uuids::uuid uuid = boost::uuids::random_generator()();
    std::string uuidStr = boost::lexical_cast<std::string>(uuid);
    psql::insert_job_in in { requestData.jobId.value(), requestData.jobName };
    
    database::transaction txn = db->startTransaction();
    psql::insertJob(txn, now, uuidStr.c_str(), in);
    txn.commit();
    return response_data { 0, requestData.jobId.value() };
  }
};

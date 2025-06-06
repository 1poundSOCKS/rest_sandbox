#pragma once

#include <database.h>

struct book_job_request
{
  std::optional<int64_t> jobId;
  std::string jobName;
  unsigned int duration;
};

struct book_job_response
{
  int64_t code;
  int64_t jobId;
};

inline void operator >>(nlohmann::json requestJson, book_job_request& request)
{
  if( requestJson.contains("body"))
  {
    auto&& body = requestJson["body"];

    if( body.is_object() )
    {
      request.jobId = body.contains("job_id") ? body["job_id"] : std::optional<int64_t>();
      request.jobName = body["job_name"];
      request.duration = body["duration"];
    }
  }
}

inline void operator <<(nlohmann::json& responseJson, std::optional<book_job_response> response)
{
  if( response.has_value() )
  {
    responseJson["header"]["code"] = response->code;
    responseJson["body"]["job_id"] = response->jobId;
  }
}

inline std::optional<book_job_response> bookJob(std::shared_ptr<database> db, transaction trx, book_job_request requestData)
{
  psql::insert_job_in in { requestData.jobId.value(), requestData.jobName, requestData.duration };  
  database::transaction txn = db->startTransaction();
  psql::insertJob(txn, trx.time, trx.id.c_str(), in);
  txn.commit();
  return book_job_response { 0, requestData.jobId.value() };
}

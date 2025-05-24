#pragma once

#include "commands/get_job_cmd.h"
#include "session.h"

class response_json
{
public:

  void operator()(const book_job_response_data& responseData);
  void operator()(get_job_cmd::response_data responseData);
  void operator()(const char* responseMessage);
  std::string toString() const;

private:

  nlohmann::json m_responseJson;

};

inline void response_json::operator()(const book_job_response_data& responseData)
{
  m_responseJson["code"] = responseData.code;
  m_responseJson["job_id"] = responseData.jobId;
}

inline void response_json::operator()(get_job_cmd::response_data responseData)
{
  m_responseJson["code"] = responseData.code;
  m_responseJson["transaction_timestamp"] = time_t_to_string(responseData.transactionTime);
  m_responseJson["transaction_id"] = responseData.transactionId;
  if( responseData.jobId.has_value() ) m_responseJson["job_id"] = responseData.jobId.value();
  if( responseData.jobName.has_value() ) m_responseJson["job_name"] = responseData.jobName.value();
}

std::string response_json::toString() const
{
  return to_string(m_responseJson);
}

inline void response_json::operator()(const char* responseMessage)
{
  m_responseJson["code"] = 999;
  m_responseJson["message"] = responseMessage;
}

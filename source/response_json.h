#pragma once

#include "session.h"

class response_json
{
public:

  void operator()(const book_job_response_data& responseData);
  std::string toString() const;

private:
  nlohmann::json m_responseJson;

};

void response_json::operator()(const book_job_response_data& responseData)
{
  m_responseJson["job_dd"] = responseData.jobId;
}

std::string response_json::toString() const
{
  return to_string(m_responseJson);
}

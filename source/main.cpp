#include "pch.h"
#include "async_connection_handler.h"

boost::beast::http::response<boost::beast::http::string_body> ProcessRequest(boost::asio::io_context& ioc, boost::beast::http::request<boost::beast::http::string_body>& request);
boost::beast::http::response<boost::beast::http::string_body> FormatErrorResponse(boost::beast::http::request<boost::beast::http::string_body>& req);

std::atomic<bool> running(true);

void handle_signal(int signal)
{
    std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
    running = false;
}

int main(int argc, char* argv[])
{
  std::signal(SIGINT, handle_signal);  // Ctrl+C
  std::signal(SIGTERM, handle_signal); // docker stop

  try
  {
    std::ifstream ifs("config.json");
    nlohmann::json config = nlohmann::json::parse(ifs);
    int port = config["port"];

    boost::asio::io_context ioc(1);
    boost::asio::ip::tcp::resolver resolver(ioc);

    async_connection_handler::start(port, [](boost::asio::io_context& ioc, std::shared_ptr<async_connection_handler::session_data> sessionData)
    {
      sessionData->response = ProcessRequest(ioc, sessionData->request);
    });

    while (running)
    {
      std::cout << "Running..." << std::endl;
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    async_connection_handler::stop();

    std::cout << "Terminating" << std::endl;
  }
  catch (const std::exception& e)
  {
    std::cout << "Error: " << e.what() << std::endl;
  }

  return 0;
}

boost::beast::http::response<boost::beast::http::string_body> ProcessRequest(boost::asio::io_context& ioc, 
  boost::beast::http::request<boost::beast::http::string_body>& req)
{
  try
  {
    nlohmann::json requestJson = nlohmann::json::parse(req.body());
    // int jobId = requestJson["job_id"];

    // insertJob.data.jobId = jobId;

    // std::string responseString = insertJob.execute() ? "Data inserted successfully." : get_sql_error(SQL_HANDLE_STMT, insertJob);
    std::string responseString = "Hello, world!";
    
    boost::beast::http::response<boost::beast::http::string_body> res(boost::beast::http::status::ok, req.version());
    res.set(boost::beast::http::field::server, "Beast");
    res.set(boost::beast::http::field::content_type, "text/json");
    res.keep_alive(req.keep_alive());

    res.body() = responseString;
    res.prepare_payload();
    return res;
  }
  catch( boost::system::system_error& err )
  {
    std::cout << err.what() << "\n";
    return FormatErrorResponse(req);
  }
  catch( ... )
  {
    std::cout << "exception\n";
    return FormatErrorResponse(req);
  }
}

boost::beast::http::response<boost::beast::http::string_body> FormatErrorResponse(boost::beast::http::request<boost::beast::http::string_body>& req)
{
  boost::beast::http::response<boost::beast::http::string_body> res(boost::beast::http::status::ok, req.version());
  res.set(boost::beast::http::field::server, "Beast");
  res.set(boost::beast::http::field::content_type, "text/json");
  res.keep_alive(req.keep_alive());

  nlohmann::json responseJson = 
  {
      { "error", "exception getting response" }
  };

  res.body() = to_string(responseJson);
  res.prepare_payload();
  return res;
}

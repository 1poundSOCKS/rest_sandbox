#include "pch.h"
#include "async_connection_handler.h"
#include "session.h"

void run(std::shared_ptr<session> s);
boost::beast::http::response<boost::beast::http::string_body> ProcessRequest(boost::asio::io_context& ioc, boost::beast::http::request<boost::beast::http::string_body>& request, std::shared_ptr<session> s);
boost::beast::http::response<boost::beast::http::string_body> FormatErrorResponse(boost::beast::http::request<boost::beast::http::string_body>& req);

std::atomic<bool> running(true);

static constexpr int g_port = 8080;
std::string g_responseBody = "Hello, world!";

void handle_signal(int signal)
{
    std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
    running = false;
}

int main(int argc, char* argv[])
{
  if( argc != 2 )
  {
    std::cout << "usage: rest_sandbox <directory>\n";
    exit(0);
  }

  if( ::chdir(argv[1]) != 0)
  {
    std::cout << "failed to set working directory to: " << argv[1] << "\n";
    exit(0);
  }
  else
  {
    std::cout << "working directory changed to: " << argv[1] << "\n";
  }

  std::signal(SIGINT, handle_signal);  // Ctrl+C
  std::signal(SIGTERM, handle_signal); // docker stop

  std::shared_ptr<session> s;

  try
  {
    std::ifstream inputFile("config.json");
    nlohmann::json config;
    inputFile >> config;
    std::string dbConnection = config["db_connection"];
    std::cout << "db connection: " << dbConnection << "\n";

    s = std::make_shared<session>(dbConnection.c_str());

    std::cout << "database version: " << s->dbVersion() << "\n";

    run(s);
  }
  catch (const std::exception& e)
  {
    std::cout << "Error: " << e.what() << std::endl;
  }
  catch (...)
  {
    std::cout << "Unknown exception caught\n";
  }

  return 0;
}

void run(std::shared_ptr<session> s)
{
  try
  {
    boost::asio::io_context ioc(1);
    boost::asio::ip::tcp::resolver resolver(ioc);

    async_connection_handler::start<std::shared_ptr<session>>(g_port, s, [](boost::asio::io_context& ioc, std::shared_ptr<async_connection_handler::session_data> sessionData, std::shared_ptr<session> s)
    {
      sessionData->response = ProcessRequest(ioc, sessionData->request, s);
    });

    std::cout << "Running..." << std::endl;

    while (running)
    {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    std::cout << "Stopped" << std::endl;

    async_connection_handler::stop();

    std::cout << "Terminating" << std::endl;
  }
  catch (const std::exception& e)
  {
    std::cout << "Error: " << e.what() << std::endl;
  }
  catch (...)
  {
    std::cout << "Unknown exception caught\n";
  }
}

boost::beast::http::response<boost::beast::http::string_body> ProcessRequest(boost::asio::io_context& ioc, 
  boost::beast::http::request<boost::beast::http::string_body>& req, std::shared_ptr<session> s)
{
  try
  {
    nlohmann::json requestJson = nlohmann::json::parse(req.body());

    std::string command = requestJson["header"]["command"];
    std::optional<nlohmann::json> responseJson;

    std::cout << "Command: " << command << "\n";

    if( command == "book-job" )
    {
      book_job_request requestData;
      requestJson >> requestData;
      auto responseData = s->run(requestData);
      responseJson = responseData.has_value() ? formatResponse(responseData.value()) : std::optional<nlohmann::json>();
    }
    else if( command == "get-job" )
    {
      get_job_request requestData;
      requestJson >> requestData;
      auto responseData = s->run(requestData);
      responseJson = responseData.has_value() ? formatResponse(responseData.value()) : std::optional<nlohmann::json>();
    }
    else
    {
      responseJson = nlohmann::json();
      responseJson.value()["code"] = 999;
      responseJson.value()["message"] = "invalid command";
    }

    boost::beast::http::response<boost::beast::http::string_body> res(boost::beast::http::status::ok, req.version());
    res.set(boost::beast::http::field::server, "Beast");
    res.set(boost::beast::http::field::content_type, "text/json");
    res.keep_alive(req.keep_alive());
    res.body() = to_string(responseJson.value());
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

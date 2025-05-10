#include "pch.h"
#include "async_connection_handler.h"

boost::beast::http::response<boost::beast::http::string_body> ProcessRequest(boost::asio::io_context& ioc, boost::beast::http::request<boost::beast::http::string_body>& request);
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
  std::signal(SIGINT, handle_signal);  // Ctrl+C
  std::signal(SIGTERM, handle_signal); // docker stop

  try
  {
    pqxx::connection conn("host=localhost port=5432 dbname=mydb user=myuser password=mypassword");

    if( conn.is_open() )
    {
      std::cout << "open\n";
      pqxx::work txn(conn);
      pqxx::result r = txn.exec("SELECT version();");
      std::cout << "PostgreSQL version: " << r[0][0].as<std::string>() << std::endl;
      txn.commit();
    }
  }
  catch (const std::exception& e)
  {
    std::cout << "Error: " << e.what() << std::endl;
  }
  catch (...)
  {
    std::cout << "Unknown exception caught\n";
  }

  try
  {
    boost::asio::io_context ioc(1);
    boost::asio::ip::tcp::resolver resolver(ioc);

    async_connection_handler::start(g_port, [](boost::asio::io_context& ioc, std::shared_ptr<async_connection_handler::session_data> sessionData)
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
  catch (...)
  {
    std::cout << "Unknown exception caught\n";
  }

  return 0;
}

boost::beast::http::response<boost::beast::http::string_body> ProcessRequest(boost::asio::io_context& ioc, 
  boost::beast::http::request<boost::beast::http::string_body>& req)
{
  try
  {
    nlohmann::json requestJson = nlohmann::json::parse(req.body());

    int id = requestJson["id"];
    std::string name = requestJson["name"];

    try
    {
      pqxx::connection conn("host=localhost port=5432 dbname=mydb user=myuser password=mypassword");

      if( conn.is_open() )
      {
        std::cout << "open\n";

        pqxx::work txn(conn);
        txn.exec_params("INSERT INTO jobs(id,name) VALUES ($1,$2)", id, name);
        txn.commit();
      }
    }
    catch (const std::exception& e)
    {
      std::cout << "Error: " << e.what() << std::endl;
    }
    catch (...)
    {
      std::cout << "Unknown exception caught\n";
    }

    std::string responseString = g_responseBody;
    
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

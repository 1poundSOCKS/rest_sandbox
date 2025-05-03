#include "pch.h"
#include "async_connection_handler.h"
#include "sql_wrappers.h"

boost::beast::http::response<boost::beast::http::string_body> ProcessRequest(boost::asio::io_context& ioc, boost::beast::http::request<boost::beast::http::string_body>& request, boost::asio::ip::tcp::endpoint& endPoint, sql_statements::insert_job& insertJob);
boost::beast::http::response<boost::beast::http::dynamic_body> CallServer(boost::asio::io_context& ioc, boost::asio::ip::tcp::endpoint& endpoint);
boost::beast::http::response<boost::beast::http::string_body> FormatErrorResponse(boost::beast::http::request<boost::beast::http::string_body>& req);

static constexpr char host[] = "httpbin.org";
static constexpr char port[] = "80";

int main(int argc, char* argv[])
{
  try
  {
    SQLRETURN ret;

    sql_handle sqlHandle;
    sqlHandle.setEnvAttr();

    SQLCHAR connStr[] = "DSN=MySQL;DATABASE=mySchema;";

    dbc_handle dbcHandle(sqlHandle);
    dbcHandle.connect(connStr);

    if( !dbcHandle.isConnected() )
    {
      std::cout << "Failed to connect to database." << std::endl;
      show_error(SQL_HANDLE_DBC, dbcHandle);
      return 0;
    }

    std::cout << "Connected to database." << std::endl;

    sql_statements::insert_job insertJob(dbcHandle);

    boost::asio::io_context ioc(1);
    boost::asio::ip::tcp::resolver resolver(ioc);
    auto hostIterator = resolver.resolve(host, port);
    auto endpoint = std::begin(hostIterator)->endpoint();

    async_connection_handler::start(8080, [&endpoint,&insertJob](boost::asio::io_context& ioc, std::shared_ptr<async_connection_handler::session_data> sessionData)
    {
      sessionData->response = ProcessRequest(ioc, sessionData->request, endpoint, insertJob);
    });

    std::string input;
    std::getline(std::cin, input);

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
  boost::beast::http::request<boost::beast::http::string_body>& req, boost::asio::ip::tcp::endpoint& endpoint,
  sql_statements::insert_job& insertJob)
{
  try
  {
    nlohmann::json requestJson = nlohmann::json::parse(req.body());
    int jobId = requestJson["job_id"];

    insertJob.data.jobId = jobId;

    std::string responseString = insertJob.execute() ? "Data inserted successfully." : get_sql_error(SQL_HANDLE_STMT, insertJob);

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

boost::beast::http::response<boost::beast::http::dynamic_body> CallServer(boost::asio::io_context& ioc, boost::asio::ip::tcp::endpoint& endpoint)
{
  boost::beast::tcp_stream stream(ioc);
  stream.connect(endpoint);

  boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get, "/ip", 10};
  req.set(boost::beast::http::field::host, host);
  req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);

  boost::beast::http::write(stream, req);

  boost::beast::flat_buffer buffer;
  boost::beast::http::response<boost::beast::http::dynamic_body> res;
  boost::beast::http::read(stream, buffer, res);

  std::cout << res << "\n";

  return res;
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

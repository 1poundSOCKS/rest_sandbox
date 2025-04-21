#include "pch.h"
#include "async_connection_handler.h"

boost::beast::http::response<boost::beast::http::string_body> ProcessRequest(boost::asio::io_context& ioc, 
  boost::beast::http::request<boost::beast::http::string_body>& req);

boost::beast::http::response<boost::beast::http::dynamic_body> CallServer(boost::asio::io_context& ioc);

boost::beast::http::response<boost::beast::http::string_body> FormatErrorResponse(boost::beast::http::request<boost::beast::http::string_body>& req);

int main(int argc, char* argv[])
{
  try
  {
    async_connection_handler::run(ProcessRequest);
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

    auto response = CallServer(ioc);
    auto responseString = boost::beast::buffers_to_string(response.body().data());

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

boost::beast::http::response<boost::beast::http::dynamic_body> CallServer(boost::asio::io_context& ioc)
{
  std::string host = "httpbin.org";
  std::string port = "80";

  boost::asio::ip::tcp::resolver resolver(ioc);
  auto hostIterator = resolver.resolve(host, port);

  auto endpoint = std::begin(hostIterator)->endpoint();

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

  nlohmann::json responseJson = 
  {
      { "error", "exception getting response" }
  };

  res.body() = to_string(responseJson);
  res.prepare_payload();
  return res;
}

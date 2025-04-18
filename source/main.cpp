#include "pch.h"

void AcceptConnection(boost::asio::io_context& ioc, boost::asio::ip::tcp::acceptor& acceptor);
void ReadRequest(boost::asio::io_context& ioc, boost::asio::ip::tcp::acceptor& acceptor, boost::asio::ip::tcp::socket& socket);
void WriteResponse(boost::asio::io_context& ioc, boost::asio::ip::tcp::acceptor& acceptor, boost::asio::ip::tcp::socket& socket, boost::beast::http::request<boost::beast::http::string_body>& req);

boost::beast::http::response<boost::beast::http::string_body> FormatResponse(boost::asio::io_context& ioc, boost::beast::http::request<boost::beast::http::string_body>& req);
boost::beast::http::response<boost::beast::http::string_body> FormatErrorResponse(boost::beast::http::request<boost::beast::http::string_body>& req);
boost::beast::http::response<boost::beast::http::dynamic_body> CallServer(boost::asio::io_context& ioc);

int main(int argc, char* argv[])
{
  try
  {
    auto const address =  boost::asio::ip::make_address("0.0.0.0");
    unsigned short port = 8080;

    boost::asio::io_context ioc(1);

    boost::beast::error_code ec;

    boost::asio::ip::tcp::acceptor acceptor(boost::asio::make_strand(ioc));

    auto endpoint = boost::asio::ip::tcp::endpoint(address, port);

    acceptor.open(endpoint.protocol(), ec);
    if (ec)
    {
        std::cout << "Set option error: " << ec.message() << std::endl;
        return 0;
    }

    acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
    if (ec)
    {
        std::cout << "Set option error: " << ec.message() << std::endl;
        return 0;
    }

    acceptor.bind(endpoint, ec);
    if (ec)
    {
        std::cout << "Bind error: " << ec.message() << std::endl;
        return 0;
    }

    acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
    if (ec)
    {
        std::cout  << "Listen error: " << ec.message() << std::endl;
        return 0;
    }

    AcceptConnection(ioc, acceptor);
    AcceptConnection(ioc, acceptor);
    AcceptConnection(ioc, acceptor);

    ioc.run();

    std::cout << "Terminating" << std::endl;
  }
  catch (const std::exception& e)
  {
    std::cout << "Error: " << e.what() << std::endl;
  }

  return 0;
}

void AcceptConnection(boost::asio::io_context& ioc, boost::asio::ip::tcp::acceptor& acceptor)
{
  acceptor.async_accept(boost::asio::make_strand(ioc), [&ioc,&acceptor](boost::beast::error_code ec, boost::asio::ip::tcp::socket socket)
  {
    std::cout << "Connection accepted" << std::endl;
    ReadRequest(ioc, acceptor, socket);
    std::cout << "request read\n";
    ioc.run();
  });
}

void ReadRequest(boost::asio::io_context& ioc, boost::asio::ip::tcp::acceptor& acceptor, boost::asio::ip::tcp::socket& socket)
{
  boost::beast::flat_buffer buffer;
  boost::beast::http::request<boost::beast::http::string_body> req;

  boost::beast::http::async_read(socket, buffer, req, [&ioc,&acceptor,&socket,&req](boost::beast::error_code ec, std::size_t bytes)
  {
    if (ec)
    {
      std::cout << "read error" << std::endl;
    }
    else
    {
      std::cout << "read success: " << bytes << "\n";
      WriteResponse(ioc, acceptor, socket, req);
      std::cout << "Response written\n";
    }
  });

  ioc.run();
}

void WriteResponse(boost::asio::io_context& ioc, boost::asio::ip::tcp::acceptor& acceptor, boost::asio::ip::tcp::socket& socket, boost::beast::http::request<boost::beast::http::string_body>& req)
{
    boost::beast::http::response<boost::beast::http::string_body> res { FormatResponse(ioc, req) };

    boost::beast::http::async_write(socket, res, [&ioc,&acceptor,&socket](boost::beast::error_code ec, std::size_t)
    {
      ec.failed() ? std::cout << "async write failed\n" : std::cout << "async write succeeded\n";
      socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
      AcceptConnection(ioc, acceptor);
    });

    ioc.run();
}

boost::beast::http::response<boost::beast::http::string_body> FormatResponse(boost::asio::io_context& ioc, boost::beast::http::request<boost::beast::http::string_body>& req)
{
  try
  {
    nlohmann::json requestJson = nlohmann::json::parse(req.body());

    auto response = CallServer(ioc);
    auto responseBody = response.body();
    auto responseString = boost::beast::buffers_to_string(responseBody.data());
  
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

  nlohmann::json responseJson = 
  {
      { "error", "exception getting response" }
  };

  res.body() = to_string(responseJson);
  res.prepare_payload();
  return res;
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

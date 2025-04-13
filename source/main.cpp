#include "pch.h"

boost::beast::flat_buffer buffer;
boost::beast::http::request<boost::beast::http::string_body> req;

void AcceptConnection(boost::asio::io_context& ioc, boost::asio::ip::tcp::acceptor& acceptor);
void AsyncRead(boost::asio::ip::tcp::socket& socket);

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

    bool loop = true;

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

    AcceptConnection(ioc, acceptor);
    AsyncRead(socket);
    ioc.run();
  });
}

void AsyncRead(boost::asio::ip::tcp::socket& socket)
{
  boost::beast::http::async_read(socket, buffer, req, [](boost::beast::error_code ec, std::size_t bytes)
  {
    if (ec)
    {
      std::cout << "read error" << std::endl;
    }
    else
    {
      std::cout << "read success: " << bytes << std::endl;
    }
  });
}

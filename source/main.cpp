#include "pch.h"

int main(int argc, char* argv[])
{
  try
  {
    auto const address =  boost::asio::ip::make_address("0.0.0.0");
    unsigned short port = 8080;

    boost::asio::io_context ioc{1};

    boost::beast::error_code ec;
    boost::asio::ip::tcp::acceptor acceptor(boost::asio::make_strand(ioc));

    auto endpoint = boost::asio::ip::tcp::endpoint{address, port};

    acceptor.open(endpoint.protocol(), ec);
    if (ec)
    {
        std::cerr << "Open error: " << ec.message() << std::endl;
        return;
    }
  }
  catch (const std::exception& e)
  {
    std::cerr << "Error: " << e.what() << std::endl;
  }

  return 0;
}

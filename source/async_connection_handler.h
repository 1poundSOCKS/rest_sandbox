#include <pch.h>

namespace async_connection_handler
{
  struct session_data
  {
    session_data(boost::asio::ip::tcp::socket& socket) : socket(std::move(socket))
    {
    }

    boost::asio::ip::tcp::socket socket;
    boost::beast::flat_buffer buffer;
    boost::beast::http::request<boost::beast::http::string_body> request;
    boost::beast::http::response<boost::beast::http::string_body> response;
  };
  
  void AcceptConnection(boost::asio::io_context& ioc, boost::asio::ip::tcp::acceptor& acceptor, auto&& processRequest);

  int run(auto&& requestHandler)
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
        std::cout << "Set option error: " << ec.message() << "\n";
        return 0;
    }

    acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
    if (ec)
    {
        std::cout << "Set option error: " << ec.message() << "\n";
        return 0;
    }

    acceptor.bind(endpoint, ec);
    if (ec)
    {
        std::cout << "Bind error: " << ec.message() << "\n";
        return 0;
    }

    acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
    if (ec)
    {
        std::cout  << "Listen error: " << ec.message() << "\n";
        return 0;
    }

    std::thread worker([&ioc,&acceptor,&requestHandler]()
    {
      AcceptConnection(ioc, acceptor, requestHandler);
      ioc.run();
    });

    std::string input;
    std::getline(std::cin, input);

    ioc.stop();
    worker.join();
    return 0;
  }

  void AcceptConnection(boost::asio::io_context& ioc, boost::asio::ip::tcp::acceptor& acceptor, auto&& processRequest)
  {
    acceptor.async_accept(boost::asio::make_strand(ioc), [&ioc,&acceptor,&processRequest](boost::beast::error_code ec, boost::asio::ip::tcp::socket socket)
    {
      std::cout << "Connection accepted\n";

      std::shared_ptr<session_data> sessionData = std::make_shared<session_data>(socket);

      boost::beast::http::async_read(sessionData->socket, sessionData->buffer, sessionData->request, [&ioc,&acceptor,&processRequest,sessionData](boost::beast::error_code ec, std::size_t bytes)
      {
        ec.failed() ? std::cout << "read error\n" : std::cout  << "read success: " << bytes << " bytes\n";
        
        sessionData->response = processRequest(ioc, sessionData->request);
        
        boost::beast::http::async_write(sessionData->socket, sessionData->response, [&ioc,&acceptor,&processRequest,sessionData](boost::beast::error_code ec, std::size_t)
        {
          ec.failed() ? std::cout << "async write failed\n" : std::cout << "async write succeeded\n";
          sessionData->socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
          AcceptConnection(ioc, acceptor, processRequest);
        });
      });
    });
  }

};

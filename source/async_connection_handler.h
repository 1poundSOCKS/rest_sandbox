#include <pch.h>

namespace async_connection_handler
{
  struct global_data
  {
    global_data(int hint) : ioc(hint), acceptor(boost::asio::make_strand(ioc))
    {
    }

    boost::asio::io_context ioc;
    boost::asio::ip::tcp::acceptor acceptor;
    std::unique_ptr<std::thread> worker;
  };

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
  
  template <typename custom_data_type, typename callback_type> void acceptConnection(boost::asio::io_context& ioc, boost::asio::ip::tcp::acceptor& acceptor, custom_data_type customData, callback_type processRequest);

  std::shared_ptr<global_data> globalData;

  template <typename custom_data_type, typename callback_type> int start(unsigned short port, custom_data_type customData, callback_type requestHandler)
  {
    auto const address =  boost::asio::ip::make_address("0.0.0.0");

    globalData = std::make_shared<global_data>(1);

    boost::beast::error_code ec;

    auto endpoint = boost::asio::ip::tcp::endpoint(address, port);

    globalData->acceptor.open(endpoint.protocol(), ec);
    if (ec)
    {
        std::cout << "Set option error: " << ec.message() << "\n";
        return 0;
    }

    globalData->acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
    if (ec)
    {
        std::cout << "Set option error: " << ec.message() << "\n";
        return 0;
    }

    globalData->acceptor.bind(endpoint, ec);
    if (ec)
    {
        std::cout << "Bind error: " << ec.message() << "\n";
        return 0;
    }

    globalData->acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
    if (ec)
    {
        std::cout  << "Listen error: " << ec.message() << "\n";
        return 0;
    }

    globalData->worker = std::make_unique<std::thread>([customData,requestHandler]()
    {
      acceptConnection<custom_data_type, callback_type>(globalData->ioc, globalData->acceptor, customData, requestHandler);
      globalData->ioc.run();
    });

    return 0;
  }

  void stop()
  {
    globalData->ioc.stop();
    globalData->worker->join();
    globalData.reset();
  }

  template <typename custom_data_type, typename callback_type> void acceptConnection(boost::asio::io_context& ioc, boost::asio::ip::tcp::acceptor& acceptor, custom_data_type customData, callback_type processRequest)
  {
    acceptor.async_accept(boost::asio::make_strand(ioc), [&ioc,&acceptor,customData,processRequest](boost::beast::error_code ec, boost::asio::ip::tcp::socket socket)
    {
      if( ec.failed() )
      {
        std::cout << "accept failed\n";
        acceptConnection<custom_data_type, callback_type>(ioc, acceptor, customData, processRequest);
        return;
      }

      std::cout << "Connection accepted\n";

      acceptConnection<custom_data_type, callback_type>(ioc, acceptor, customData, processRequest);

      std::shared_ptr<session_data> sessionData = std::make_shared<session_data>(socket);

      boost::beast::http::async_read(sessionData->socket, sessionData->buffer, sessionData->request, [&ioc,&acceptor,processRequest,sessionData,customData](boost::beast::error_code ec, std::size_t bytes)
      {
        if( ec.failed() )
        {
          std::cout << "read error\n";
          acceptConnection<custom_data_type, callback_type>(ioc, acceptor, customData, processRequest);
          return;
        }

        std::cout  << "read success: " << bytes << " bytes\n";

        processRequest(ioc, sessionData, customData);

        boost::beast::http::async_write(sessionData->socket, sessionData->response, [&ioc,sessionData](boost::beast::error_code ec, std::size_t)
        {
          ec.failed() ? std::cout << "async write failed\n" : std::cout << "async write succeeded\n";
          sessionData->socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
        });
      });
    });
  }
};

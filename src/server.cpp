// ./boost_1_81_0/doc/html/boost_asio/example/cpp11/echo/async_tcp_echo_server.cpp

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

struct session
  : public std::enable_shared_from_this<session>
{
  session(tcp::socket socket)
    : socket(std::move(socket)) { }

  void start() {
    do_read();
  }

  void do_read() {
    auto self(shared_from_this());
    memset(data, 0, sizeof(data));
    socket.async_read_some(boost::asio::buffer(data, max_length),
      [this, self](boost::system::error_code ec, std::size_t length) {
        if (!ec) {
          std::cout << data << "\n";
          do_write(length);
          sleep(1); //test
        } else {
          std::cout << ec.message() << "\n";
        }
      });
  }

  void do_write(std::size_t length)
  {
    auto self(shared_from_this());
    boost::asio::async_write(socket, boost::asio::buffer("hello client \n"),
        [this, self](boost::system::error_code ec, std::size_t /*length*/)
        {
          if (!ec) {
            do_read();
          } else {
            std::cout << ec.message() << "\n";
          }
        });
  }

  tcp::socket socket;
  enum { max_length = 1024 };
  char data[max_length];
};

struct server {
  server(boost::asio::io_context& io_context, short port)
    : acceptor(io_context, tcp::endpoint(tcp::v4(), port))
  {
    std::cout << "listen on port: " << port << " \n";
    do_accept();
  }

  void do_accept() {
    acceptor.async_accept(
      [this](boost::system::error_code ec, tcp::socket socket) {
        if (!ec) {
          std::cout << "accept connection: "
            << socket.remote_endpoint() << "\n";
          std::make_shared<session>(std::move(socket))->start();
        } else {
          std::cout << ec.message() << ", " <<
            socket.remote_endpoint();
        }
        do_accept();
      });
  }

  tcp::acceptor acceptor;
};

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: server <port>\n";
    return 1;
  }

  boost::asio::io_context io_context;
  server s(io_context, std::atoi(argv[1]));
  io_context.run();
  return 0;
}

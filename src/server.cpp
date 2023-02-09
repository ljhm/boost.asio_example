// ./boost_1_81_0/doc/html/boost_asio/example/cpp11/echo/async_tcp_echo_server.cpp

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <sanitizer/lsan_interface.h>

using boost::asio::ip::tcp;

void handlerCont(int signum){
  if (signum == SIGCONT) {
    printf("Got SIGCONT\n");
  }
#ifndef NDEBUG
  __lsan_do_recoverable_leak_check();
#endif
}

struct session
  : public std::enable_shared_from_this<session>
{
  session(tcp::socket socket)
    : socket(std::move(socket)) { }

  void start() {
    start_read();
    start_write();
  }

  void start_read() {
    auto self(shared_from_this());
    memset(data, 0, sizeof(data));
    socket.async_read_some(boost::asio::buffer(data, max_length),
      [this, self](boost::system::error_code ec, std::size_t length) {
        if (!ec) {
          handle_read(ec, length);
        } else {
          std::cout << ec.message() << "\n";
        }
      });
  }

  void handle_read(const boost::system::error_code& error,
    std::size_t n) {
    if (!error) {
      std::cout << data;
      start_read();
    } else {
      std::cout << error.message() << "\n";
    }
  }

  void start_write() {
    auto self(shared_from_this());
    boost::asio::async_write(socket,
      boost::asio::buffer(msg + (std::to_string(cnt++) + "\n").c_str()),
      [this, self](boost::system::error_code ec, std::size_t /*length*/)
      {
        if (!ec) {
          handle_write(ec);
        } else {
          std::cout << ec.message() << "\n";
        }
      });
  }

  void handle_write(const boost::system::error_code& error) {
    if (!error) {
      sleep(1); //test
      start_write();
    } else {
      std::cout << error.message() << "\n";
    }
  }

  tcp::socket socket;
  enum { max_length = 1024 };
  char data[max_length];
  std::string msg = "hello client ";
  size_t cnt = 0;
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
            socket.remote_endpoint() << "\n";
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

  signal(SIGCONT, handlerCont); // $ man 7 signal
  boost::asio::io_context io_context;
  server s(io_context, std::atoi(argv[1]));
  io_context.run();

  return 0;
}


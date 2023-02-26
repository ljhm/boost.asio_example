// ./boost_1_81_0/doc/html/boost_asio/example/cpp11/echo/async_tcp_echo_server.cpp

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <sanitizer/lsan_interface.h>

struct session
  : public std::enable_shared_from_this<session>
{
  session(boost::asio::ip::tcp::socket socket)
    : socket(std::move(socket))
  { }

  void start() {
    start_read();
    start_write();
  }

  void start_read() {
    auto self(shared_from_this());
    memset(input_data, 0, sizeof(input_data));
    socket.async_read_some(
      boost::asio::buffer(input_data, sizeof(input_data)),
      [&, self](boost::system::error_code ec, std::size_t length) {
        if (!ec) {
          std::cout << input_data;
          start_read();
        } else {
          std::cout << ec.message() << "\n";
        }
      }
    );
  }

  void start_write() {
    auto self(shared_from_this());
    memset(output_data, 0, sizeof(output_data));
    snprintf(output_data, sizeof(output_data) - 1,
      "hello client %zu\n", cnt++);
    boost::asio::async_write(
      socket,
      boost::asio::buffer(output_data, sizeof(input_data)),
      [&, self](boost::system::error_code ec, std::size_t length)
      {
        if (!ec) {
          // sleep(1); //test
          start_write();
        } else {
          std::cout << ec.message() << "\n";
        }
      }
    );
  }

  boost::asio::ip::tcp::socket socket;
  enum { LEN = 1024 };
  char input_data[LEN];
  char output_data[LEN];
  size_t cnt = 0;
};

struct server {
  server(boost::asio::io_context& io_context, short port)
    : acceptor(io_context, boost::asio::ip::tcp::endpoint(
        boost::asio::ip::tcp::v4(), port))
  {
    std::cout << "Listen on port: " << port << " \n";
    do_accept();
  }

  void do_accept() {
    acceptor.async_accept(
      [&](boost::system::error_code ec,
        boost::asio::ip::tcp::socket socket)
      {
        if (!ec) {
          std::cout << "Accept connection: "
            << socket.remote_endpoint() << "\n";
          std::make_shared<session>(std::move(socket))->start();
        } else {
          std::cout << ec.message() << "\n";
        }
        do_accept();
      }
    );
  }

  boost::asio::ip::tcp::acceptor acceptor;
};

void handlerCont(int signum) {
  printf("SIGCONT %d\n", signum);
#ifndef NDEBUG
  __lsan_do_recoverable_leak_check();
#endif
}

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


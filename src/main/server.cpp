
// https://github.com/chriskohlhoff/asio/blob/master/asio/src/examples/cpp11/echo/async_tcp_echo_server.cpp

#include <asio.hpp>
#include <iostream>

#ifndef NDEBUG
#include <sanitizer/lsan_interface.h>
#endif

struct session : public std::enable_shared_from_this<session> {
  session(asio::ip::tcp::socket socket) : socket(std::move(socket)) {}

  void start() {
    start_read();
    start_write();
  }

  void start_read() {
    auto self(shared_from_this());
    memset(input_data, 0, sizeof(input_data));
    asio::async_read(socket, asio::buffer(input_data, sizeof(input_data)),
                     [this, self](std::error_code ec, std::size_t length) {
                       if (!ec) {
                         std::cout << input_data;
                         start_read(); // continuous read test with recursion
                       } else {
                         std::cout << ec.message() << "\n"; // error
                       }
                     });
  }

  void start_write() {
    auto self(shared_from_this());
    memset(output_data, 0, sizeof(output_data));
    snprintf(output_data, sizeof(output_data) - 1, "hello client %zu\n", cnt++);
    asio::async_write(socket, asio::buffer(output_data, sizeof(output_data)),
                      [this, self](std::error_code ec, std::size_t length) {
                        if (!ec) {
                          start_write(); // continuous write test with recursion
                        } else {
                          std::cout << ec.message() << "\n"; // error
                        }
                      });
  }

  asio::ip::tcp::socket socket;
  enum { LEN = 1024 };
  char input_data[LEN];
  char output_data[LEN];
  size_t cnt = 0;
};

struct server {
  server(asio::io_context &io_context, short port)
      : acceptor(io_context,
                 asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port)) {
    std::cout << "Listen on port: " << port << " \n";
    do_accept();
  }

  void do_accept() {
    acceptor.async_accept([this](std::error_code ec,
                                 asio::ip::tcp::socket socket) {
      if (!ec) {
        std::cout << "Accept connection: " << socket.remote_endpoint() << "\n";
        std::make_shared<session>(std::move(socket))->start();
      } else {
        std::cout << ec.message() << "\n";
      }
      do_accept();
    });
  }

  asio::ip::tcp::acceptor acceptor;
};

void handlerCont(int signum) {
  printf("SIGCONT %d\n", signum);
#ifndef NDEBUG
  __lsan_do_recoverable_leak_check();
#endif
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: server <port>\n";
    return 1;
  }

  signal(SIGCONT, handlerCont); // kill -CONT 123 # pid

  asio::io_context io_context;
  server s(io_context, std::atoi(argv[1]));
  io_context.run();

  return 0;
}

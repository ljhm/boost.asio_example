
// https://github.com/chriskohlhoff/asio/blob/master/asio/src/examples/cpp11/timeouts/async_tcp_client.cpp

#include <asio.hpp>
#include <iostream>
#include <string>

#ifndef NDEBUG
#include <sanitizer/lsan_interface.h>
#endif

std::string client_tag; // test

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
    snprintf(output_data, sizeof(output_data) - 1, "hello server %s %zu\n",
             client_tag.c_str(), cnt++);
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

struct client {
  client(asio::io_context &io_context,
         asio::ip::tcp::resolver::results_type endpoints)
      : socket(io_context), endpoints(endpoints) {
    do_connect(endpoints.begin());
  }

  void
  do_connect(asio::ip::tcp::resolver::results_type::iterator endpoint_iter) {
    if (endpoint_iter != endpoints.end()) {
      socket.async_connect(
          endpoint_iter->endpoint(), [&, this](const std::error_code ec) {
            if (!socket.is_open()) {
              std::cout << "Connect timed out\n";
              do_connect(++endpoint_iter);
            } else if (ec) {
              std::cout << "Connect error: " << ec.message() << "\n";
              socket.close();
            } else {
              std::cout << "Connected to " << socket.remote_endpoint() << "\n";
              std::make_shared<session>(std::move(socket))->start();
            }
          });
    }
  }

  asio::ip::tcp::resolver::results_type endpoints;
  asio::ip::tcp::socket socket;
};

void handlerCont(int signum) {
  printf("SIGCONT %d\n", signum);
#ifndef NDEBUG
  __lsan_do_recoverable_leak_check();
#endif
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    std::cerr << "Usage: client <host> <port> <tag>\n";
    return 1;
  }

  signal(SIGCONT, handlerCont); // kill -CONT 123 # pid
  client_tag = argv[3];

  asio::io_context io_context;
  asio::ip::tcp::resolver r(io_context);
  client c(io_context, r.resolve(argv[1], argv[2]));
  io_context.run();

  return 0;
}

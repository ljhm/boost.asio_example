// ./boost_1_81_0/doc/html/boost_asio/example/cpp11/timeouts/async_tcp_client.cpp

#include <iostream>
#include <string>
#include <boost/asio.hpp>

#ifndef NDEBUG
#include <sanitizer/lsan_interface.h>
#endif

std::string client_tag; //test

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
    boost::asio::async_read(
      socket,
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
      "hello server %s %zu\n", client_tag.c_str(), cnt++);
    boost::asio::async_write(
      socket,
      boost::asio::buffer(output_data, sizeof(output_data)),
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

struct client {
  client(boost::asio::io_context& io_context,
    boost::asio::ip::tcp::resolver::results_type endpoints)
      : socket(io_context), endpoints(endpoints)
  {
    do_connect(endpoints.begin());
  }

  void do_connect (
    boost::asio::ip::tcp::resolver::results_type::iterator
    endpoint_iter)
  {
    if (endpoint_iter != endpoints.end()) {
      socket.async_connect(
        endpoint_iter->endpoint(),
        [&](const boost::system::error_code ec)
        {
          if (!socket.is_open()) {
            std::cout << "Connect timed out\n";
            do_connect(++endpoint_iter);
          } else if (ec) {
            std::cout << "Connect error: " << ec.message() << "\n";
            socket.close();
          } else {
            std::cout << "Connected to " <<
              socket.remote_endpoint() << "\n";
            std::make_shared<session>(std::move(socket))->start();
          }
        }
      );
    }
  }

  boost::asio::ip::tcp::resolver::results_type endpoints;
  boost::asio::ip::tcp::socket socket;
};

void handlerCont(int signum){
  printf("SIGCONT %d\n", signum);
#ifndef NDEBUG
  __lsan_do_recoverable_leak_check();
#endif
}

int main(int argc, char* argv[]) {
  if (argc != 4) {
    std::cerr << "Usage: client <host> <port> <tag>\n";
    return 1;
  }

  signal(SIGCONT, handlerCont); // kill -CONT 123 # pid
  client_tag = argv[3];

  boost::asio::io_context io_context;
  boost::asio::ip::tcp::resolver r(io_context);
  client c(io_context, r.resolve(argv[1], argv[2]));
  io_context.run();

  return 0;
}

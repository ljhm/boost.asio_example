// ./boost_1_81_0/doc/html/boost_asio/example/cpp11/timeouts/async_tcp_client.cpp

#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/write.hpp>
#include <functional>
#include <iostream>
#include <string>

using boost::asio::steady_timer;
using boost::asio::ip::tcp;
using std::placeholders::_1;
using std::placeholders::_2;

struct client {
  client(boost::asio::io_context& io_context)
    : socket(io_context) { }

  void start(tcp::resolver::results_type endpoints,
    const std::string& msg)
  {
    endpoints_ = endpoints;
    msg_ = msg;
    start_connect(endpoints_.begin());
  }

  void start_connect(tcp::resolver::results_type::iterator
    endpoint_iter)
  {
    if (endpoint_iter != endpoints_.end()) {
      socket.async_connect(endpoint_iter->endpoint(),
        std::bind(&client::handle_connect,
          this, _1, endpoint_iter));
    }
  }

  void handle_connect(const boost::system::error_code& error,
      tcp::resolver::results_type::iterator endpoint_iter)
  {
    if (!socket.is_open()) {
      std::cout << "Connect timed out\n";
      start_connect(++endpoint_iter);
    } else if (error) {
      std::cout << "Connect error: " << error.message() << "\n";
      socket.close();
      start_connect(++endpoint_iter);
    } else {
      std::cout << "Connected to " << endpoint_iter->endpoint() << "\n";

      //do I have to write before read?
      start_write();
      start_read();
    }
  }

  void start_write() {
    boost::asio::async_write(socket,
      boost::asio::buffer("hello server " + msg_ + " "),
      std::bind(&client::handle_write, this, _1));
  }

  void handle_write(const boost::system::error_code& error) {
    if (!error) {
      start_write();
      sleep(1); //test
    } else {
      std::cout << error.message() << "\n";
    }
  }

  void start_read() {
    boost::asio::async_read_until(socket,
      boost::asio::dynamic_buffer(input_buffer_), '\n',
      std::bind(&client::handle_read, this, _1, _2));
  }

  void handle_read(const boost::system::error_code& error,
    std::size_t n)
  {
    if (!error) {
      std::string line(input_buffer_.substr(0, n - 1));
      input_buffer_.erase(0, n);

      if (!line.empty()) {
        std::cout << line << "\n";
      }

      start_read();//
    } else {
      std::cout << error.message() << "\n";
    }
  }

  tcp::resolver::results_type endpoints_;
  tcp::socket socket;
  std::string input_buffer_;
  std::string msg_;
};

int main(int argc, char* argv[])
{
  if (argc != 4) {
    std::cerr << "Usage: client <host> <port> <msg>\n";
    return 1;
  }

  boost::asio::io_context io_context;
  tcp::resolver r(io_context);
  client c(io_context);

  c.start(r.resolve(argv[1], argv[2]), argv[3]);

  io_context.run();

  return 0;
}

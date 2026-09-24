// Throwaway Phase 0 practice: one client, one message, one response.
// Goal: understand the async callback lifecycle before Phase 3 needs it for real.
#include <boost/asio.hpp>
#include <iostream>
#include <memory>

using boost::asio::ip::tcp;

// Owns the socket and buffer for one connection's lifetime.
// Held alive via shared_ptr passed into each callback -- if we let this
// object die before a callback fires, the callback touches freed memory.
class Session : public std::enable_shared_from_this<Session> {
public:
    explicit Session(tcp::socket socket) : socket_(std::move(socket)) {}

    void start() { do_read(); }

private:
    void do_read() {
        auto self = shared_from_this(); // keep *this* alive until the handler runs
        boost::asio::async_read_until(
            socket_, buffer_, '\n',
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (ec) {
                    std::cerr << "[server] read error: " << ec.message() << "\n";
                    return;
                }
                std::istream is(&buffer_);
                std::string line;
                std::getline(is, line);
                std::cout << "[server] received: " << line << "\n";
                do_write("ack: " + line + "\n");
            });
    }

    void do_write(std::string response) {
        auto self = shared_from_this();
        auto msg = std::make_shared<std::string>(std::move(response));
        boost::asio::async_write(
            socket_, boost::asio::buffer(*msg),
            [this, self, msg](boost::system::error_code ec, std::size_t) {
                if (ec) {
                    std::cerr << "[server] write error: " << ec.message() << "\n";
                }
                // Connection is done after one round trip -- socket closes
                // when `self` (the last shared_ptr) goes out of scope here.
            });
    }

    tcp::socket socket_;
    boost::asio::streambuf buffer_;
};

class Server {
public:
    Server(boost::asio::io_context& io, unsigned short port)
        : acceptor_(io, tcp::endpoint(tcp::v4(), port)) {
        do_accept();
    }

private:
    void do_accept() {
        acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                std::cout << "[server] connection accepted\n";
                std::make_shared<Session>(std::move(socket))->start();
            }
            do_accept(); // keep listening for the next connection
        });
    }

    tcp::acceptor acceptor_;
};

int main() {
    try {
        boost::asio::io_context io;
        Server server(io, 5555);
        std::cout << "[server] listening on port 5555\n";
        io.run(); // blocks, dispatching handlers as events arrive
    } catch (std::exception& e) {
        std::cerr << "[server] exception: " << e.what() << "\n";
        return 1;
    }
    return 0;
}

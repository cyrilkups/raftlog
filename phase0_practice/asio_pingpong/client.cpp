// Throwaway Phase 0 practice: connects, sends one message, reads the response.
#include <boost/asio.hpp>
#include <iostream>

using boost::asio::ip::tcp;

int main() {
    try {
        boost::asio::io_context io;

        tcp::socket socket(io);
        tcp::resolver resolver(io);
        boost::asio::connect(socket, resolver.resolve("127.0.0.1", "5555"));

        std::string message = "hello from client\n";
        boost::asio::write(socket, boost::asio::buffer(message));

        boost::asio::streambuf response_buf;
        boost::asio::read_until(socket, response_buf, '\n');

        std::istream is(&response_buf);
        std::string response;
        std::getline(is, response);
        std::cout << "[client] server replied: " << response << "\n";
    } catch (std::exception& e) {
        std::cerr << "[client] exception: " << e.what() << "\n";
        return 1;
    }
    return 0;
}

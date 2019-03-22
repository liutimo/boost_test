#include <iostream>
#include "examples_buffer.h"


int main() {
    try {
        boost::asio::io_context io_context;
        server s(io_context, 1313);
        io_context.run();
    }catch (std::exception &ec) {
        std::cout << ec.what() << std::endl;
    }




    return 0;
}
#include "Server.hpp"
#include <iostream>
#include <cstdlib>
#include <signal.h>

Server* g_server = NULL;

void signalHandler(int signum) {
    (void)signum;
    if (g_server) {
        std::cout << "\nShutting down server..." << std::endl;
        g_server->stop();
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }

    std::string port = argv[1];
    std::string password = argv[2];

    // Set up signal handler
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    try {
        Server server(port, password);
        g_server = &server;

        std::cout << "IRC Server started on port " << port << std::endl;
        server.run();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

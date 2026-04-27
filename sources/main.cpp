#include "../includes/Server.hpp"
#include <iostream>
#include <cstdlib>
#include <cctype>
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

    bool passwordIsBlank = password.empty();
    if (!passwordIsBlank) {
        passwordIsBlank = true;
        for (size_t i = 0; i < password.size(); ++i) {
            if (!std::isspace(static_cast<unsigned char>(password[i]))) {
                passwordIsBlank = false;
                break;
            }
        }
    }

    if (passwordIsBlank) {
        std::cerr << "Error: password must not be empty or whitespace" << std::endl;
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }

    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGPIPE, SIG_IGN);

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

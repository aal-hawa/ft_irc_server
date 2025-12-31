#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "Commands.hpp"
#include "Utils.hpp"
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>

Server::Server(const std::string& port, const std::string& password)
    : _port(port),
      _password(password),
      _serverSocket(-1),
      _running(false) {

    _hostname = "localhost";
    _creationTime = Utils::getCurrentTime();

    createSocket();
    setNonBlocking(_serverSocket);
    listenForConnections();

    _pollFds.push_back((struct pollfd){_serverSocket, POLLIN, 0});
}

Server::~Server() {
    stop();

    // Close all client connections
    for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        delete it->second;
    }
    _clients.clear();

    // Delete all channels
    for (std::vector<Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
        delete *it;
    }
    _channels.clear();

    // Close server socket
    if (_serverSocket != -1) {
        close(_serverSocket);
    }
}

void Server::run() {
    _running = true;
    runPollLoop();
}

void Server::stop() {
    _running = false;
}

std::string Server::getPort() const {
    return _port;
}

std::string Server::getPassword() const {
    return _password;
}

std::string Server::getHostname() const {
    return _hostname;
}

void Server::addClient(Client* client) {
    if (client) {
        _clients[client->getFd()] = client;
        _pollFds.push_back((struct pollfd){client->getFd(), POLLIN, 0});
    }
}

void Server::removeClient(int fd) {
    // Remove from poll fds
    for (size_t i = 0; i < _pollFds.size(); ++i) {
        if (_pollFds[i].fd == fd) {
            _pollFds.erase(_pollFds.begin() + i);
            break;
        }
    }

    // Remove from clients map
    std::map<int, Client*>::iterator it = _clients.find(fd);
    if (it != _clients.end()) {
        delete it->second;
        _clients.erase(it);
    }
}

Client* Server::getClientByFd(int fd) const {
    std::map<int, Client*>::const_iterator it = _clients.find(fd);
    if (it != _clients.end()) {
        return it->second;
    }
    return NULL;
}

Client* Server::getClientByNickname(const std::string& nickname) const {
    for (std::map<int, Client*>::const_iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second->getNickname() == nickname) {
            return it->second;
        }
    }
    return NULL;
}

void Server::addChannel(Channel* channel) {
    if (channel) {
        _channels.push_back(channel);
    }
}

void Server::removeChannel(Channel* channel) {
    for (std::vector<Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
        if (*it == channel) {
            _channels.erase(it);
            break;
        }
    }
}

Channel* Server::getChannel(const std::string& name) const {
    for (std::vector<Channel*>::const_iterator it = _channels.begin(); it != _channels.end(); ++it) {
        if ((*it)->getName() == name) {
            return *it;
        }
    }
    return NULL;
}

std::vector<Channel*> Server::getChannelsByClient(Client* client) const {
    std::vector<Channel*> result;
    for (std::vector<Channel*>::const_iterator it = _channels.begin(); it != _channels.end(); ++it) {
        if ((*it)->isMember(client)) {
            result.push_back(*it);
        }
    }
    return result;
}

void Server::broadcastToChannel(Channel* channel, const std::string& message, Client* exclude) {
    const std::map<int, Client*>& members = channel->getMembers();
    for (std::map<int, Client*>::const_iterator it = members.begin(); it != members.end(); ++it) {
        if (it->second != exclude) {
            it->second->sendToClient(message);
        }
    }
}

void Server::sendWelcome(Client* client) {
    client->sendToClient("001 " + client->getNickname() + " :Welcome to the Internet Relay Network " + client->getPrefix());
    client->sendToClient("002 " + client->getNickname() + " :Your host is " + _hostname + ", running version ft_irc");
    client->sendToClient("003 " + client->getNickname() + " :This server was created " + _creationTime);
    client->sendToClient("004 " + client->getNickname() + " " + _hostname + " ft_irc i o");
}

void Server::sendNames(Client* client, Channel* channel) {
    std::string namesList;
    const std::map<int, Client*>& members = channel->getMembers();

    for (std::map<int, Client*>::const_iterator it = members.begin(); it != members.end(); ++it) {
        Client* member = it->second;
        if (channel->isOperator(member)) {
            namesList += "@";
        }
        namesList += member->getNickname() + " ";
    }

    client->sendToClient("353 " + client->getNickname() + " = " + channel->getName() + " :" + namesList);
    client->sendToClient("366 " + client->getNickname() + " " + channel->getName() + " :End of /NAMES list");
}

void Server::createSocket() {
    int port = Utils::atoi(_port);

    _serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverSocket == -1) {
        throw std::runtime_error("Failed to create socket");
    }

    int opt = 1;
    if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        close(_serverSocket);
        throw std::runtime_error("Failed to set socket options");
    }

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(_serverSocket, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        close(_serverSocket);
        throw std::runtime_error("Failed to bind socket");
    }
}

void Server::setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        close(_serverSocket);
        throw std::runtime_error("Failed to get socket flags");
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        close(_serverSocket);
        throw std::runtime_error("Failed to set non-blocking mode");
    }
}

void Server::listenForConnections() {
    if (listen(_serverSocket, 10) == -1) {
        close(_serverSocket);
        throw std::runtime_error("Failed to listen on socket");
    }
}

void Server::acceptNewConnection() {
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    int clientFd = accept(_serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
    if (clientFd == -1) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            // Error accepting connection
        }
        return;
    }

    setNonBlocking(clientFd);

    Client* client = new Client(clientFd, "unknown", this);
    addClient(client);
}

void Server::runPollLoop() {
    while (_running) {
        int pollResult = poll(&_pollFds[0], _pollFds.size(), -1);

        if (pollResult == -1) {
            if (errno == EINTR) {
                continue;
            }
            break;
        }

        for (size_t i = 0; i < _pollFds.size(); ++i) {
            if (_pollFds[i].revents & POLLIN) {
                if (_pollFds[i].fd == _serverSocket) {
                    acceptNewConnection();
                } else {
                    handleClientData(_pollFds[i].fd);
                }
            }
        }
    }
}

void Server::handleClientData(int clientFd) {
    char buffer[1024];
    ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);

    if (bytesRead <= 0) {
        handleClientDisconnect(clientFd);
        return;
    }

    buffer[bytesRead] = '\0';

    Client* client = getClientByFd(clientFd);
    if (client) {
        client->appendRecvBuffer(buffer);

        while (client->hasCompleteMessage()) {
            std::string messageStr = client->getNextMessage();
            if (!messageStr.empty()) {
                processCommand(client, messageStr);
            }
        }
    }
}

void Server::handleClientDisconnect(int clientFd) {
    Client* client = getClientByFd(clientFd);
    if (client) {
        Command_QUIT(this, client, Message("QUIT :Client disconnected"));
    }
}

void Server::processCommand(Client* client, const std::string& messageStr) {
    Message message(messageStr);

    if (!message.isComplete()) {
        return;
    }

    std::string command = Utils::toUpper(message.getCommand());

    if (command == "PASS") {
        Command_PASS(this, client, message);
    } else if (command == "NICK") {
        Command_NICK(this, client, message);
    } else if (command == "USER") {
        Command_USER(this, client, message);
    } else if (command == "JOIN") {
        Command_JOIN(this, client, message);
    } else if (command == "PART") {
        Command_PART(this, client, message);
    } else if (command == "PRIVMSG") {
        Command_PRIVMSG(this, client, message);
    } else if (command == "QUIT") {
        Command_QUIT(this, client, message);
    } else if (command == "KICK") {
        Command_KICK(this, client, message);
    } else if (command == "INVITE") {
        Command_INVITE(this, client, message);
    } else if (command == "TOPIC") {
        Command_TOPIC(this, client, message);
    } else if (command == "MODE") {
        Command_MODE(this, client, message);
    } else {
        client->sendToClient("421 " + command + " :Unknown command");
    }
}

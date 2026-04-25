#ifndef SERVER_HPP
#define SERVER_HPP

#include "Client.hpp"
#include "Channel.hpp"

#include <string>
#include <vector>
#include <map>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class Client;
class Channel;

class Server {
private:
    int _serverSocket;
    std::string _port;
    std::string _password;
    std::string _hostname;
    std::string _creationTime;
    std::vector<struct pollfd> _pollFds;
    std::map<int, Client*> _clients;
    std::vector<Channel*> _channels;
    bool _running;

public:
    Server(const std::string& port, const std::string& password);
    ~Server();

    void run();
    void stop();

    std::string getPort() const;
    std::string getPassword() const;
    std::string getHostname() const;

    void addClient(Client* client);
    void removeClient(int fd);
    Client* getClientByFd(int fd) const;
    Client* getClientByNickname(const std::string& nickname) const;

    void addChannel(Channel* channel);
    void removeChannel(Channel* channel);
    Channel* getChannel(const std::string& name) const;
    std::vector<Channel*> getChannelsByClient(Client* client) const;

    void broadcastToChannel(Channel* channel, const std::string& message, Client* exclude);

    void sendWelcome(Client* client);
    void sendNames(Client* client, Channel* channel);

    void flushClientOutput(int clientFd);

private:
    void createSocket();
    void setNonBlocking(int fd);
    void listenForConnections();
    void acceptNewConnection();
    void runPollLoop();
    void handleClientData(int clientFd);
    void handleClientDisconnect(int clientFd);
    void processCommand(Client* client, const std::string& message);
};

#endif

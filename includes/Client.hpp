#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <deque>

class Server;

class Client {
private:
    int _fd;
    std::string _nickname;
    std::string _username;
    std::string _hostname;
    std::string _realname;
    std::string _recvBuffer;
    std::deque<std::string> _sendBuffer;
    bool _isRegistered;
    bool _isOperator;
    bool _isAuthenticated;
    Server* _server;

public:
    Client(int fd, const std::string& hostname, Server* server);
    ~Client();

    // Getters
    int getFd() const;
    std::string getNickname() const;
    std::string getUsername() const;
    std::string getHostname() const;
    std::string getRealname() const;
    bool isRegistered() const;
    bool isOperator() const;
    bool isAuthenticated() const;
    std::string getPrefix() const;

    // Setters
    void setNickname(const std::string& nick);
    void setUsername(const std::string& username);
    void setRealname(const std::string& realname);
    void setRegistered(bool registered);
    void setOperator(bool isOp);
    void setAuthenticated(bool auth);

    // Message handling
    void sendToClient(const std::string& message);
    void appendRecvBuffer(const std::string& data);
    bool hasCompleteMessage() const;
    std::string getNextMessage();
};

#endif

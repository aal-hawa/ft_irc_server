#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <deque>
#include <ctime>

#include <unistd.h>
#include <cstring>


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
    bool _isAuthenticated;
    time_t _lastActivity;

    bool _waitingPong;
    time_t _lastPingTime;

public:
    Client(int fd, const std::string& hostname);
    ~Client();

    int getFd() const;
    std::string getNickname() const;
    std::string getUsername() const;
    std::string getHostname() const;
    std::string getRealname() const;
    bool isRegistered() const;
    bool isAuthenticated() const;
    std::string getPrefix() const;

    void setNickname(const std::string& nick);
    void setUsername(const std::string& username);
    void setRealname(const std::string& realname);
    void setRegistered(bool registered);
    void setAuthenticated(bool auth);

    void sendToClient(const std::string& message);
    void appendRecvBuffer(const std::string& data);
    bool hasCompleteMessage() const;
    std::string getNextMessage();

    bool hasPendingOutput() const;
    std::string& frontSendBuffer();
    void popFrontSendBuffer();

    void updateLastActivity();
    time_t getLastActivity() const;

    bool isWaitingPong() const;
    void setWaitingPong(bool value);

    time_t getLastPingTime() const;
    void setLastPingTime(time_t timeValue);
};

#endif

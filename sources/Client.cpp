#include "../includes/Client.hpp"
#include <iostream>


Client::Client(int fd, const std::string& hostname)
    : _fd(fd),
      _hostname(hostname),
      _isRegistered(false),
      _isAuthenticated(false),
      _lastActivity(std::time(NULL)),
      _waitingPong(false),
      _lastPingTime(0) {
}

Client::~Client() {
    if (_fd != -1) {
        close(_fd);
        _fd = -1;
    }
}

int Client::getFd() const {
    return _fd;
}

std::string Client::getNickname() const {
    return _nickname;
}

std::string Client::getUsername() const {
    return _username;
}

std::string Client::getHostname() const {
    return _hostname;
}

std::string Client::getRealname() const {
    return _realname;
}

bool Client::isRegistered() const {
    return _isRegistered;
}



bool Client::isAuthenticated() const {
    return _isAuthenticated;
}

std::string Client::getPrefix() const {
    std::string nick = _nickname.empty() ? "*" : _nickname;
    std::string user = _username.empty() ? "*" : _username;
    return nick + "!" + user + "@" + _hostname;
}

void Client::setNickname(const std::string& nick) {
    _nickname = nick;
}

void Client::setUsername(const std::string& username) {
    _username = username;
}

void Client::setRealname(const std::string& realname) {
    _realname = realname;
}

void Client::setRegistered(bool registered) {
    _isRegistered = registered;
}



void Client::setAuthenticated(bool auth) {
    _isAuthenticated = auth;
}

void Client::sendToClient(const std::string& message) {
    _sendBuffer.push_back(message + "\r\n");
}

void Client::appendRecvBuffer(const std::string& data) {
    _recvBuffer += data;
}

bool Client::hasCompleteMessage() const {
    return (_recvBuffer.find("\r\n") != std::string::npos)
        || (_recvBuffer.find("\n") != std::string::npos);
}

std::string Client::getNextMessage()
{
    size_t pos = _recvBuffer.find("\r\n");
    size_t delimLen = 2;

    if (pos == std::string::npos)
    {
        pos = _recvBuffer.find("\n");
        delimLen = 1;
    }

    if (pos == std::string::npos)
    return "";

    std::string msg = _recvBuffer.substr(0, pos);
   _recvBuffer.erase(0, pos + delimLen);

    if (!msg.empty() && msg[msg.size() - 1] == '\r')
        msg.erase(msg.size() -1);
    
    return msg + "\r\n";
}

bool Client::hasPendingOutput() const {
    return !_sendBuffer.empty();
}

std::string& Client::frontSendBuffer() {
    return _sendBuffer.front();
}

void Client::popFrontSendBuffer() {
    _sendBuffer.pop_front();
}

void Client::updateLastActivity() {
    _lastActivity = std::time(NULL);
}

time_t Client::getLastActivity() const {
    return _lastActivity;
}

bool Client::isWaitingPong() const {
    return _waitingPong;
}

void Client::setWaitingPong(bool value) {
    _waitingPong = value;
}

time_t Client::getLastPingTime() const {
    return _lastPingTime;
}

void Client::setLastPingTime(time_t timeValue) {
    _lastPingTime = timeValue;
}
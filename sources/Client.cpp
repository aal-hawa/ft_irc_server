#include "../includes/Client.hpp"
// #include "../includes/Server.hpp"


// Client::Client(int fd, const std::string& hostname, Server* server)
Client::Client(int fd, const std::string& hostname)
    : _fd(fd),
      _hostname(hostname),
      _isRegistered(false),
      _isOperator(false),
    //   _isAuthenticated(false),
    //   _server(server) {

      _isAuthenticated(false) {
    char host[NI_MAXHOST];
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    if (getpeername(_fd, (struct sockaddr*)&addr, &addr_len) == 0) {
        getnameinfo((struct sockaddr*)&addr, addr_len, host, sizeof(host), NULL, 0, NI_NUMERICHOST);
        _hostname = host;
    }
    // (void)server; // To avoid unused parameter warning if _server is not used
}

Client::~Client() {
    if (_fd != -1) {
        close(_fd);
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

bool Client::isOperator() const {
    return _isOperator;
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

void Client::setOperator(bool isOp) {
    _isOperator = isOp;
}

void Client::setAuthenticated(bool auth) {
    _isAuthenticated = auth;
}

void Client::sendToClient(const std::string& message) {
    std::string fullMessage = message + "\r\n";
    ssize_t sent = send(_fd, fullMessage.c_str(), fullMessage.length(), 0);

    if (sent < 0) {
        // Error sending, connection may be closed
    }
}

void Client::appendRecvBuffer(const std::string& data) {
    _recvBuffer += data;
}

bool Client::hasCompleteMessage() const {
    return _recvBuffer.find("\r\n") != std::string::npos;
}

std::string Client::getNextMessage() {
    size_t pos = _recvBuffer.find("\r\n");
    if (pos != std::string::npos) {
        std::string message = _recvBuffer.substr(0, pos);
        _recvBuffer = _recvBuffer.substr(pos + 2);
        return message;
    }
    return "";
}

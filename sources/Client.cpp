#include "../includes/Client.hpp"

Client::Client(int fd, const std::string& hostname)
    : _fd(fd),
      _hostname(hostname),
      _isRegistered(false),
      _isOperator(false),
      _isAuthenticated(false) {
    char host[NI_MAXHOST];
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);

    if (getpeername(_fd, (struct sockaddr*)&addr, &addr_len) == 0) {
        getnameinfo((struct sockaddr*)&addr, addr_len, host, sizeof(host), NULL, 0, NI_NUMERICHOST);
        _hostname = host;
    }
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

// FIXED: Buffer messages instead of sending directly
// This allows poll() to monitor POLLOUT before sending
void Client::sendToClient(const std::string& message) {
    std::string fullMessage = message + "\r\n";
    _sendBuffer.push_back(fullMessage);
}

// FIXED: Support both \r\n (telnet) and \n (nc)
bool Client::hasCompleteMessage() const {
    return _recvBuffer.find("\r\n") != std::string::npos ||
           _recvBuffer.find("\n") != std::string::npos;
}

// FIXED: Support both \r\n (telnet) and \n (nc)
std::string Client::getNextMessage() {
    size_t pos = _recvBuffer.find("\r\n");
    size_t lineLen = 2;  // Length of \r\n

    // If no \r\n, check for just \n (for nc compatibility)
    if (pos == std::string::npos) {
        pos = _recvBuffer.find("\n");
        lineLen = 1;  // Length of \n
    }

    if (pos != std::string::npos) {
        std::string message = _recvBuffer.substr(0, pos + lineLen);
        _recvBuffer = _recvBuffer.substr(pos + lineLen);
        return message;
    }
    return "";
}

void Client::appendRecvBuffer(const std::string& data) {
    _recvBuffer += data;
}

// Check if there's data waiting to be sent
bool Client::hasPendingData() const {
    return !_sendBuffer.empty();
}

// FIXED: Only call send() when poll() indicates POLLOUT is ready
void Client::flushSendBuffer() {
    while (!_sendBuffer.empty()) {
        const std::string& msg = _sendBuffer.front();
        ssize_t sent = send(_fd, msg.c_str(), msg.length(), MSG_NOSIGNAL);

        if (sent < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Socket not ready for writing, keep data in buffer
                // poll() will notify us when POLLOUT is ready
                return;
            }
            // Error occurred, remove the message to avoid infinite loop
            _sendBuffer.pop_front();
            return;
        }

        if (static_cast<size_t>(sent) < msg.length()) {
            // Partial send, keep remainder in buffer
            _sendBuffer.front() = msg.substr(sent);
            return;
        }

        // Full message sent, remove from buffer
        _sendBuffer.pop_front();
    }
}

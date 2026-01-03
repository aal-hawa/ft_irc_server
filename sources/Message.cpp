#include "../includes/Message.hpp"
#include <algorithm>

Message::Message(const std::string& raw) : _raw(raw), _complete(true) {
    if (_raw.find("\r\n") == std::string::npos) {
        _complete = false;
    } else {
        // Remove \r\n from the end
        size_t crlfPos = _raw.find("\r\n");
        _raw = _raw.substr(0, crlfPos);
        parse();
    }
}

void Message::parse() {
    extractPrefix();
    extractCommandAndParams();
}

bool Message::isComplete() const {
    return _complete;
}

std::string Message::getPrefix() const {
    return _prefix;
}

std::string Message::getCommand() const {
    return _command;
}

std::vector<std::string> Message::getParams() const {
    return _params;
}

std::string Message::getTrailing() const {
    return _trailing;
}

void Message::extractPrefix() {
    if (_raw.empty() || _raw[0] != ':') {
        return;
    }

    size_t spacePos = _raw.find(' ');
    if (spacePos != std::string::npos) {
        _prefix = _raw.substr(1, spacePos - 1);
        _raw = _raw.substr(spacePos + 1);
    } else {
        _raw = "";
    }
}

void Message::extractCommandAndParams() {
    if (_raw.empty()) {
        return;
    }

    // Extract command
    size_t spacePos = _raw.find(' ');
    if (spacePos != std::string::npos) {
        _command = _raw.substr(0, spacePos);
        _raw = _raw.substr(spacePos + 1);
    } else {
        _command = _raw;
        _raw = "";
        return;
    }

    // Extract params
    while (!_raw.empty()) {
        if (_raw[0] == ':') {
            // Trailing parameter
            _trailing = _raw.substr(1);
            _params.push_back(_trailing);
            break;
        }

        spacePos = _raw.find(' ');
        if (spacePos != std::string::npos) {
            std::string param = _raw.substr(0, spacePos);
            _params.push_back(param);
            _raw = _raw.substr(spacePos + 1);
        } else {
            if (!_raw.empty()) {
                _params.push_back(_raw);
            }
            break;
        }
    }
}

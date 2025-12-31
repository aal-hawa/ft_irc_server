#include "Utils.hpp"
#include <ctime>
#include <algorithm>
#include <cctype>

bool Utils::isValidPassword(const std::string& password) {
    return !password.empty();
}

bool Utils::isValidNickname(const std::string& nickname) {
    if (nickname.empty() || nickname.length() > 9) {
        return false;
    }

    // First character must be letter or special chars: [ \ ] ^ _ `
    if (!isalpha(nickname[0]) &&
        nickname[0] != '[' && nickname[0] != ']' &&
        nickname[0] != '\\' && nickname[0] != '^' &&
        nickname[0] != '_' && nickname[0] != '`') {
        return false;
    }

    // Rest can be letter, digit, or special chars: - [ \ ] ^ _ `
    for (size_t i = 1; i < nickname.length(); ++i) {
        if (!isalnum(nickname[i]) &&
            nickname[i] != '-' && nickname[i] != '[' &&
            nickname[i] != ']' && nickname[i] != '\\' &&
            nickname[i] != '^' && nickname[i] != '_' &&
            nickname[i] != '`') {
            return false;
        }
    }

    return true;
}

bool Utils::isValidUsername(const std::string& username) {
    if (username.empty() || username.length() > 9) {
        return false;
    }

    // Username can contain letters, digits, and hyphens
    for (size_t i = 0; i < username.length(); ++i) {
        if (!isalnum(username[i]) && username[i] != '-') {
            return false;
        }
    }

    return true;
}

bool Utils::isValidChannelName(const std::string& channel) {
    if (channel.empty() || channel.length() > 200) {
        return false;
    }

    // Channel must start with # or &
    if (channel[0] != '#' && channel[0] != '&') {
        return false;
    }

    // Rest can be any printable character except space, comma, or control chars
    for (size_t i = 1; i < channel.length(); ++i) {
        if (channel[i] == ' ' || channel[i] == ',' || channel[i] == 7) {
            return false;
        }
        if (channel[i] < 1 || channel[i] > 31 || channel[i] == 127) {
            return false;
        }
    }

    return true;
}

std::vector<std::string> Utils::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }

    return tokens;
}

std::string Utils::toUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

void Utils::trim(std::string& str) {
    // Trim leading whitespace
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start != std::string::npos) {
        str = str.substr(start);
    }

    // Trim trailing whitespace
    size_t end = str.find_last_not_of(" \t\r\n");
    if (end != std::string::npos) {
        str = str.substr(0, end + 1);
    }
}

int Utils::atoi(const std::string& str) {
    std::stringstream ss(str);
    int result;
    ss >> result;
    return result;
}

std::string Utils::getCurrentTime() {
    time_t now = time(0);
    char* dt = ctime(&now);
    std::string timeStr(dt);
    timeStr.erase(timeStr.length() - 1); // Remove newline
    return timeStr;
}

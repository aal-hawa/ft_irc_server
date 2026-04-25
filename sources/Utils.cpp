#include "../includes/Utils.hpp"
#include <ctime>
#include <algorithm>
#include <cctype>
#include <climits>

bool Utils::isValidPassword(const std::string& password) {
    return !password.empty();
}

bool Utils::isValidNickname(const std::string& nickname) {
    if (nickname.empty() || nickname.length() > 9) {
        return false;
    }

    if (!isalpha(nickname[0]) &&
        nickname[0] != '[' && nickname[0] != ']' &&
        nickname[0] != '\\' && nickname[0] != '^' &&
        nickname[0] != '_' && nickname[0] != '`') {
        return false;
    }

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

    if (channel.length() < 2) {
        return false;
    }

    if (channel[0] != '#' && channel[0] != '&') {
        return false;
    }

    for (size_t i = 1; i < channel.length(); ++i) {
        if (channel[i] <= 32 || channel[i] == ',' || channel[i] == 127) {
            return false;
        }
    }

    return true;
}

std::string Utils::normalizeNickname(const std::string& nickname) {
    std::string normalized = nickname;
    for (size_t i = 0; i < normalized.length(); ++i) {
        if (normalized[i] >= 'A' && normalized[i] <= 'Z') {
            normalized[i] = normalized[i] - 'A' + 'a';
        } else if (normalized[i] == '{') {
            normalized[i] = '[';
        } else if (normalized[i] == '}') {
            normalized[i] = ']';
        } else if (normalized[i] == '|') {
            normalized[i] = '\\';
        } else if (normalized[i] == '~') {
            normalized[i] = '^';
        }
    }
    return normalized;
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
    size_t start = str.find_first_not_of(" \t\r\n");
    if (start != std::string::npos) {
        str = str.substr(start);
    }

    size_t end = str.find_last_not_of(" \t\r\n");
    if (end != std::string::npos) {
        str = str.substr(0, end + 1);
    }
}

int Utils::atoi(const std::string& str)
{
    std::stringstream ss(str);
    long result = 0;
    char extra;

    if (!(ss >> result) || (ss >> extra)) {
        return 0;
    }
    if (result < INT_MIN || result > INT_MAX) {
        return 0;
    }
    return static_cast<int>(result);
}


bool Utils::isPositiveNumber(const std::string& str)
{
    if (str.empty())
    {
        return false;
    }
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (!std::isdigit(static_cast<unsigned char>(str[i])))
        {
            return false;
        }
    }
    return true;
}

std::string Utils::getCurrentTime() {
    time_t now = time(0);
    char* dt = ctime(&now);
    std::string timeStr(dt);
    timeStr.erase(timeStr.length() - 1);
    return timeStr;
}

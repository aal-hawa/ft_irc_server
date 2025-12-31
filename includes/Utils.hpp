#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>
#include <sstream>

class Utils {
public:
    static bool isValidPassword(const std::string& password);
    static bool isValidNickname(const std::string& nickname);
    static bool isValidUsername(const std::string& username);
    static bool isValidChannelName(const std::string& channel);
    static std::vector<std::string> split(const std::string& str, char delimiter);
    static std::string toUpper(const std::string& str);
    static void trim(std::string& str);
    static int atoi(const std::string& str);
    static std::string getCurrentTime();
};

#endif

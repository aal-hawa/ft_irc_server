#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <vector>

class Message {
private:
    std::string _raw;
    std::string _prefix;
    std::string _command;
    std::vector<std::string> _params;
    std::string _trailing;
    bool _complete;
    bool _firstParamTrailing;

public:
    Message(const std::string& raw);

    void parse();
    bool isComplete() const;

    std::string getPrefix() const;
    std::string getCommand() const;
    std::vector<std::string> getParams() const;
    std::string getTrailing() const;
    bool firstParamWasTrailing() const;

private:
    void extractPrefix();
    void extractCommandAndParams();
    size_t paramCount() const;
};

#endif

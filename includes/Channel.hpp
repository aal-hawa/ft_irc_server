#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Client.hpp"
#include <string>
#include <map>
#include <set>
#include <ctime>

class Client;

class Channel {
private:
    std::string _name;
    std::string _topic;
    std::string _key;
    std::string _topicSetter;
    time_t _topicSetTime;
    std::map<int, Client*> _members;
    std::set<int> _operators;
    std::set<int> _invited;
    bool _inviteOnly;
    bool _topicRestricted;
    unsigned int _userLimit;

public:
    Channel(const std::string& name);
    ~Channel();

    std::string getName() const;
    std::string getTopic() const;
    std::string getKey() const;
    std::string getTopicSetter() const;
    time_t getTopicSetTime() const;
    bool isInviteOnly() const;
    bool isTopicRestricted() const;
    unsigned int getUserLimit() const;
    size_t getMemberCount() const;
    const std::map<int, Client*>& getMembers() const;

    void setTopic(const std::string& topic, const std::string& setter);
    void setKey(const std::string& key);
    void setInviteOnly(bool inviteOnly);
    void setTopicRestricted(bool restricted);
    void setUserLimit(unsigned int limit);

    void addMember(Client* client);
    void removeMember(Client* client);
    bool isMember(Client* client) const;
    bool isMember(int fd) const;

    void addOperator(Client* client);
    void removeOperator(Client* client);
    bool isOperator(Client* client) const;

    void inviteUser(Client* client);
    bool isInvited(Client* client) const;
    void uninviteUser(Client* client);
};

#endif

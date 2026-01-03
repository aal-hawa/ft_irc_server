#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "Client.hpp"
#include <string>
#include <map>
#include <set>

class Client;

class Channel {
private:
    std::string _name;
    std::string _topic;
    std::string _key;
    std::map<int, Client*> _members;
    std::set<int> _operators;
    std::set<std::string> _invited;
    bool _inviteOnly;
    bool _topicRestricted;
    unsigned int _userLimit;

public:
    Channel(const std::string& name, Client* founder);
    ~Channel();

    // Getters
    std::string getName() const;
    std::string getTopic() const;
    std::string getKey() const;
    bool isInviteOnly() const;
    bool isTopicRestricted() const;
    unsigned int getUserLimit() const;
    size_t getMemberCount() const;
    const std::map<int, Client*>& getMembers() const;

    // Setters
    void setTopic(const std::string& topic);
    void setKey(const std::string& key);
    void setInviteOnly(bool inviteOnly);
    void setTopicRestricted(bool restricted);
    void setUserLimit(unsigned int limit);

    // Member management
    void addMember(Client* client);
    void removeMember(Client* client);
    bool isMember(Client* client) const;
    bool isMember(int fd) const;

    // Operator management
    void addOperator(Client* client);
    void removeOperator(Client* client);
    bool isOperator(Client* client) const;

    // Invite management
    void inviteUser(const std::string& nickname);
    bool isInvited(const std::string& nickname) const;
    void uninviteUser(const std::string& nickname);
};

#endif

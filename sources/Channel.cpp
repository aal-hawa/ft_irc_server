#include "../includes/Channel.hpp"
#include "../includes/Client.hpp"

Channel::Channel(const std::string& name, Client* founder)
    : _name(name),
      _inviteOnly(false),
      _topicRestricted(false),
      _userLimit(0) {
    if (founder) {
        _members[founder->getFd()] = founder;
        _operators.insert(founder->getFd());
    }
}

Channel::~Channel() {
    _members.clear();
    _operators.clear();
    _invited.clear();
}

std::string Channel::getName() const {
    return _name;
}

std::string Channel::getTopic() const {
    return _topic;
}

std::string Channel::getKey() const {
    return _key;
}

bool Channel::isInviteOnly() const {
    return _inviteOnly;
}

bool Channel::isTopicRestricted() const {
    return _topicRestricted;
}

unsigned int Channel::getUserLimit() const {
    return _userLimit;
}

size_t Channel::getMemberCount() const {
    return _members.size();
}

const std::map<int, Client*>& Channel::getMembers() const {
    return _members;
}

void Channel::setTopic(const std::string& topic) {
    _topic = topic;
}

void Channel::setKey(const std::string& key) {
    _key = key;
}

void Channel::setInviteOnly(bool inviteOnly) {
    _inviteOnly = inviteOnly;
}

void Channel::setTopicRestricted(bool restricted) {
    _topicRestricted = restricted;
}

void Channel::setUserLimit(unsigned int limit) {
    _userLimit = limit;
}

void Channel::addMember(Client* client) {
    if (client) {
        _members[client->getFd()] = client;
    }
}

void Channel::removeMember(Client* client) {
    if (client) {
        _members.erase(client->getFd());
        _operators.erase(client->getFd());
    }
}

bool Channel::isMember(Client* client) const {
    if (!client) {
        return false;
    }
    return _members.find(client->getFd()) != _members.end();
}

bool Channel::isMember(int fd) const {
    return _members.find(fd) != _members.end();
}

void Channel::addOperator(Client* client) {
    if (client && isMember(client)) {
        _operators.insert(client->getFd());
    }
}

void Channel::removeOperator(Client* client) {
    if (client) {
        _operators.erase(client->getFd());
    }
}

bool Channel::isOperator(Client* client) const {
    if (!client) {
        return false;
    }
    return _operators.find(client->getFd()) != _operators.end();
}

void Channel::inviteUser(const std::string& nickname) {
    _invited.insert(nickname);
}

bool Channel::isInvited(const std::string& nickname) const {
    return _invited.find(nickname) != _invited.end();
}

void Channel::uninviteUser(const std::string& nickname) {
    _invited.erase(nickname);
}

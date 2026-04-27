#include "../includes/Commands.hpp"
#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/Message.hpp"
#include "../includes/Utils.hpp"
#include <sstream>
#include <set>

static std::string nickOrStar(Client* client)
{
    const std::string& nick = client->getNickname();
    return nick.empty() ? std::string("*") : nick;
}

static void tryRegister(Server* server, Client* client)
{
    if (client->isRegistered())
        return;
    if (!client->isAuthenticated())
        return;
    if (client->getNickname().empty())
        return;
    if (client->getUsername().empty())
        return;

    client->setRegistered(true);
    server->sendWelcome(client);
}

void Command_PING(Server* server, Client* client, const Message& message)
{
    std::vector<std::string> params = message.getParams();
    std::string token;
    if (!params.empty())
        token = params[0];
    client->sendToClient(":" + server->getHostname() + " PONG :" + token);
}

void Command_PONG(Server* server, Client* client, const Message& message)
{
    (void)server;
    (void)message;

    client->setWaitingPong(false);
    client->setLastPingTime(0);
}

void Command_CAP(Server* server, Client* client, const Message& message)
{
    std::vector<std::string> params = message.getParams();
    if (params.empty()) return;

    if (params[0] == "LS")
    {
        client->sendToClient(":" + server->getHostname() + " CAP * LS :");
    }
}

void Command_PASS(Server* server, Client* client, const Message& message)
{
    std::vector<std::string> params = message.getParams();

    if (params.empty()) {
        client->sendToClient(":" + server->getHostname() + " 461 " + nickOrStar(client) + " PASS :Not enough parameters");
        return;
    }

    if (client->isAuthenticated()) {
        client->sendToClient(":" + server->getHostname() + " 462 " + nickOrStar(client) + " :You may not reregister");
        return;
    }

    if (params[0] == server->getPassword()) {
        client->setAuthenticated(true);
        tryRegister(server, client);
    } else {
        client->sendToClient(":" + server->getHostname() + " 464 " + nickOrStar(client) + " :Password incorrect");
        client->setAuthenticated(false);
    }
}

void Command_NICK(Server* server, Client* client, const Message& message)
{
    std::vector<std::string> params = message.getParams();

    if (params.empty()) {
        client->sendToClient(":" + server->getHostname() + " 431 " + nickOrStar(client) + " :No nickname given");
        return;
    }

    if (message.firstParamWasTrailing()) {
        client->sendToClient(":" + server->getHostname() + " 432 " + nickOrStar(client) + " :* :Erroneous nickname");
        return;
    }

    if (params.size() > 1 && message.getTrailing().empty()) {
        client->sendToClient(":" + server->getHostname() + " 432 " + nickOrStar(client) + " " + params[0] + " :Erroneous nickname");
        return;
    }

    std::string newNick = params[0];

    if (!Utils::isValidNickname(newNick)) {
        client->sendToClient(":" + server->getHostname() + " 432 " + nickOrStar(client) + " " + newNick + " :Erroneous nickname");
        return;
    }

    Client* nickOwner = server->getClientByNickname(newNick);
    if (nickOwner && nickOwner != client) {
        client->sendToClient(":" + server->getHostname() + " 433 " + nickOrStar(client) + " " + newNick + " :Nickname is already in use");
        return;
    }

    std::string oldNick = client->getNickname();
    std::string oldPrefix = client->getPrefix();
    client->setNickname(newNick);

    if (client->isRegistered() && !oldNick.empty()) {
        std::string nickMsg = ":" + oldPrefix + " NICK :" + newNick;
        std::set<int> sentTo;
        std::vector<Channel*> channels = server->getChannelsByClient(client);
        for (size_t i = 0; i < channels.size(); ++i) {
            const std::map<int, Client*>& members = channels[i]->getMembers();
            for (std::map<int, Client*>::const_iterator it = members.begin(); it != members.end(); ++it) {
                if (sentTo.insert(it->first).second) {
                    it->second->sendToClient(nickMsg);
                }
            }
        }
        if (sentTo.empty()) {
            client->sendToClient(nickMsg);
        }
    }

    tryRegister(server, client);
}

void Command_USER(Server* server, Client* client, const Message& message)
{
    std::vector<std::string> params = message.getParams();

    if (params.size() < 4) {
        client->sendToClient(":" + server->getHostname() + " 461 " + nickOrStar(client) + " USER :Not enough parameters");
        return;
    }

    if (client->isRegistered()) {
        client->sendToClient(":" + server->getHostname() + " 462 " + nickOrStar(client) + " :You may not reregister");
        return;
    }

    if (!Utils::isValidUsername(params[0])) {
        client->sendToClient(":" + server->getHostname() + " 461 " + nickOrStar(client) + " USER :Invalid username");
        return;
    }

    client->setUsername(params[0]);
    client->setRealname(params[3]);

    tryRegister(server, client);
}

void Command_JOIN(Server* server, Client* client, const Message& message)
{
    std::vector<std::string> params = message.getParams();

    if (params.empty()) {
        client->sendToClient(":" + server->getHostname() + " 461 " + nickOrStar(client) + " JOIN :Not enough parameters");
        return;
    }

    std::vector<std::string> channels = Utils::split(params[0], ',');
    std::vector<std::string> keys;
    if (params.size() > 1) {
        keys = Utils::split(params[1], ',');
    }

    for (size_t i = 0; i < channels.size(); ++i) {
        std::string channelName = channels[i];
        std::string key = (i < keys.size()) ? keys[i] : "";

        if (!Utils::isValidChannelName(channelName)) {
            client->sendToClient(":" + server->getHostname() + " 403 " + nickOrStar(client) + " " + channelName + " :No such channel");
            continue;
        }

        Channel* channel = server->getChannel(channelName);
        if (!channel) {
            channel = new Channel(channelName);
            server->addChannel(channel);
        } else {
            if (channel->isMember(client)) {
                continue;
            }

            if (channel->isInviteOnly() && !channel->isInvited(client)) {
                client->sendToClient(":" + server->getHostname() + " 473 " + nickOrStar(client) + " " + channelName + " :Cannot join channel (+i)");
                continue;
            }

            if (!channel->getKey().empty() && channel->getKey() != key) {
                client->sendToClient(":" + server->getHostname() + " 475 " + nickOrStar(client) + " " + channelName + " :Cannot join channel (+k)");
                continue;
            }

            if (channel->getUserLimit() > 0 && channel->getMemberCount() >= channel->getUserLimit()) {
                client->sendToClient(":" + server->getHostname() + " 471 " + nickOrStar(client) + " " + channelName + " :Cannot join channel (+l)");
                continue;
            }
        }

        channel->addMember(client);
        channel->uninviteUser(client);

        if (channel->getMemberCount() == 1) {
            channel->addOperator(client);
        }

        client->sendToClient(":" + client->getPrefix() + " JOIN :" + channelName);

        if (!channel->getTopic().empty()) {
            client->sendToClient(":" + server->getHostname() + " 332 " +
                client->getNickname() + " " + channelName + " :" + channel->getTopic());

                if (!channel->getTopicSetter().empty() && channel->getTopicSetTime() > 0) {
                std::ostringstream oss333;
                oss333 << channel->getTopicSetTime();
                client->sendToClient(":" + server->getHostname() + " 333 " +
                    client->getNickname() + " " + channelName + " " +
                    channel->getTopicSetter() + " " + oss333.str());
            }
        } else {
            client->sendToClient(":" + server->getHostname() + " 331 " +
                client->getNickname() + " " + channelName + " :No topic is set");
        }

        server->sendNames(client, channel);
        server->broadcastToChannel(channel, ":" + client->getPrefix() + " JOIN :" + channelName, client);
    }
}

void Command_PART(Server* server, Client* client, const Message& message)
{
    std::vector<std::string> params = message.getParams();

    if (params.empty()) {
        client->sendToClient(":" + server->getHostname() + " 461 " + nickOrStar(client) + " PART :Not enough parameters");
        return;
    }

    std::vector<std::string> channels = Utils::split(params[0], ',');
    std::string reason = (params.size() > 1) ? params[1] : "";

    for (size_t i = 0; i < channels.size(); ++i) {
        std::string channelName = channels[i];
        Channel* channel = server->getChannel(channelName);

        if (!channel) {
            client->sendToClient(":" + server->getHostname() + " 403 " + nickOrStar(client) + " " + channelName + " :No such channel");
            continue;
        }

        if (!channel->isMember(client)) {
            client->sendToClient(":" + server->getHostname() + " 442 " + nickOrStar(client) + " " + channelName + " :You're not on that channel");
            continue;
        }

        std::string partMsg = ":" + client->getPrefix() + " PART " + channelName;
        if (!reason.empty()) {
            partMsg += " :" + reason;
        }
        server->broadcastToChannel(channel, partMsg, NULL);
        channel->removeMember(client);

        if (channel->getMemberCount() == 0) {
            server->removeChannel(channel);
            delete channel;
        }
    }
}

void Command_PRIVMSG(Server* server, Client* client, const Message& message)
{
    std::vector<std::string> params = message.getParams();

    if (params.empty()) {
        client->sendToClient(":" + server->getHostname() + " 411 " + nickOrStar(client) + " :No recipient given (PRIVMSG)");
        return;
    }

    if (params.size() < 2) {
        client->sendToClient(":" + server->getHostname() + " 412 " + nickOrStar(client) + " :No text to send");
        return;
    }

    std::string target = params[0];
    std::string text = params[1];

    if (target.empty()) {
        client->sendToClient(":" + server->getHostname() + " 411 " + nickOrStar(client) + " :No recipient given (PRIVMSG)");
        return;
    }

    if (target[0] == '#' || target[0] == '&') {
        Channel* channel = server->getChannel(target);
        if (!channel) {
            client->sendToClient(":" + server->getHostname() + " 403 " + nickOrStar(client) + " " + target + " :No such channel");
            return;
        }

        if (!channel->isMember(client)) {
            client->sendToClient(":" + server->getHostname() + " 404 " + nickOrStar(client) + " " + target + " :Cannot send to channel");
            return;
        }

        std::string msg = ":" + client->getPrefix() + " PRIVMSG " + target + " :" + text;
        server->broadcastToChannel(channel, msg, client);
    } else {
        Client* targetClient = server->getClientByNickname(target);
        if (!targetClient) {
            client->sendToClient(":" + server->getHostname() + " 401 " + nickOrStar(client) + " " + target + " :No such nick/channel");
            return;
        }

        std::string msg = ":" + client->getPrefix() + " PRIVMSG " + target + " :" + text;
        targetClient->sendToClient(msg);
    }
}

void Command_NOTICE(Server* server, Client* client, const Message& message)
{
    std::vector<std::string> params = message.getParams();

    if (params.empty() || params.size() < 2) {
        return;
    }

    std::string target = params[0];
    std::string text = params[1];

    if (target.empty()) {
        return;
    }

    if (target[0] == '#' || target[0] == '&') {
        Channel* channel = server->getChannel(target);
        if (!channel || !channel->isMember(client)) {
            return;
        }

        std::string msg = ":" + client->getPrefix() + " NOTICE " + target + " :" + text;
        server->broadcastToChannel(channel, msg, client);
    } else {
        Client* targetClient = server->getClientByNickname(target);
        if (!targetClient) {
            return;
        }

        std::string msg = ":" + client->getPrefix() + " NOTICE " + target + " :" + text;
        targetClient->sendToClient(msg);
    }
}

void Command_QUIT(Server* server, Client* client, const Message& message)
{
    std::string reason;
    std::vector<std::string> params = message.getParams();
    if (!params.empty()) {
        reason = params[0];
    } else {
        reason = client->getNickname();
    }

    std::string quitMsg = ":" + client->getPrefix() + " QUIT :" + reason;

    std::vector<Channel*> channels = server->getChannelsByClient(client);
    for (size_t i = 0; i < channels.size(); ++i) {
        server->broadcastToChannel(channels[i], quitMsg, client);
        channels[i]->removeMember(client);
        if (channels[i]->getMemberCount() == 0) {
            server->removeChannel(channels[i]);
            delete channels[i];
        }
    }

    server->removeClient(client->getFd());
}

void Command_KICK(Server* server, Client* client, const Message& message)
{
    std::vector<std::string> params = message.getParams();

    if (params.size() < 2) {
        client->sendToClient(":" + server->getHostname() + " 461 " + nickOrStar(client) + " KICK :Not enough parameters");
        return;
    }

    std::string channelName = params[0];
    std::string nickname = params[1];
    std::string comment = (params.size() > 2) ? params[2] : client->getNickname();

    Channel* channel = server->getChannel(channelName);
    if (!channel) {
        client->sendToClient(":" + server->getHostname() + " 403 " + nickOrStar(client) + " " + channelName + " :No such channel");
        return;
    }

    if (!channel->isOperator(client)) {
        client->sendToClient(":" + server->getHostname() + " 482 " + nickOrStar(client) + " " + channelName + " :You're not channel operator");
        return;
    }

    Client* targetClient = server->getClientByNickname(nickname);
    if (!targetClient || !channel->isMember(targetClient)) {
        client->sendToClient(":" + server->getHostname() + " 441 " + nickOrStar(client) + " " + channelName + " " + nickname + " :They aren't on that channel");
        return;
    }

    std::string kickMsg = ":" + client->getPrefix() + " KICK " + channelName + " " + nickname + " :" + comment;
    server->broadcastToChannel(channel, kickMsg, NULL);
    channel->removeMember(targetClient);

    if (channel->getMemberCount() == 0) {
        server->removeChannel(channel);
        delete channel;
    }
}

void Command_INVITE(Server* server, Client* client, const Message& message)
{
    std::vector<std::string> params = message.getParams();

    if (params.size() < 2) {
        client->sendToClient(":" + server->getHostname() + " 461 " + nickOrStar(client) + " INVITE :Not enough parameters");
        return;
    }

    std::string nickname = params[0];
    std::string channelName = params[1];

    Channel* channel = server->getChannel(channelName);
    if (!channel) {
        client->sendToClient(":" + server->getHostname() + " 403 " + nickOrStar(client) + " " + channelName + " :No such channel");
        return;
    }

    if (!channel->isMember(client)) {
        client->sendToClient(":" + server->getHostname() + " 442 " + nickOrStar(client) + " " + channelName + " :You're not on that channel");
        return;
    }

    if (channel->isInviteOnly() && !channel->isOperator(client))
    {
        client->sendToClient(":" + server->getHostname() + " 482 " + nickOrStar(client) + " " + channelName + " :You're not channel operator");
        return;
    }

    Client* targetClient = server->getClientByNickname(nickname);
    if (!targetClient) {
        client->sendToClient(":" + server->getHostname() + " 401 " + nickOrStar(client) + " " + nickname + " :No such nick");
        return;
    }

    if (channel->isMember(targetClient)) {
        client->sendToClient(":" + server->getHostname() + " 443 " + nickOrStar(client) + " " + nickname + " " + channelName + " :is already on channel");
        return;
    }

    channel->inviteUser(targetClient);
    client->sendToClient(":" + server->getHostname() + " 341 " +
        client->getNickname() + " " + nickname + " :" + channelName);
    targetClient->sendToClient(":" + client->getPrefix() + " INVITE " + nickname + " :" + channelName);
}

void Command_TOPIC(Server* server, Client* client, const Message& message)
{
    std::vector<std::string> params = message.getParams();

    if (params.empty()) {
        client->sendToClient(":" + server->getHostname() + " 461 " + nickOrStar(client) + " TOPIC :Not enough parameters");
        return;
    }

    std::string channelName = params[0];
    Channel* channel = server->getChannel(channelName);

    if (!channel) {
        client->sendToClient(":" + server->getHostname() + " 403 " + nickOrStar(client) + " " + channelName + " :No such channel");
        return;
    }

    if (!channel->isMember(client)) {
        client->sendToClient(":" + server->getHostname() + " 442 " + nickOrStar(client) + " " + channelName + " :You're not on that channel");
        return;
    }

    if (params.size() == 1) {
        if (channel->getTopic().empty()) {
            client->sendToClient(":" + server->getHostname() + " 331 " + client->getNickname() + " " + channelName + " :No topic is set");
        } else {
            client->sendToClient(":" + server->getHostname() + " 332 " + client->getNickname() + " " + channelName + " :" + channel->getTopic());
            if (!channel->getTopicSetter().empty() && channel->getTopicSetTime() > 0) {
                std::ostringstream oss333;
                oss333 << channel->getTopicSetTime();
                client->sendToClient(":" + server->getHostname() + " 333 " +
                    client->getNickname() + " " + channelName + " " +
                    channel->getTopicSetter() + " " + oss333.str());
            }
        }
        return;
    }

    if (channel->isTopicRestricted() && !channel->isOperator(client)) {
        client->sendToClient(":" + server->getHostname() + " 482 " + nickOrStar(client) + " " + channelName + " :You're not channel operator");
        return;
    }

    std::string newTopic = params[1];
    channel->setTopic(newTopic, client->getNickname());
    server->broadcastToChannel(channel, ":" + client->getPrefix() + " TOPIC " + channelName + " :" + newTopic, NULL);
}

void Command_MODE(Server* server, Client* client, const Message& message)
{
    std::vector<std::string> params = message.getParams();

    if (params.empty()) {
        client->sendToClient(":" + server->getHostname() + " 461 " + nickOrStar(client) + " MODE :Not enough parameters");
        return;
    }

    std::string target = params[0];

    if (target[0] == '#' || target[0] == '&') {
        Channel* channel = server->getChannel(target);
        if (!channel) {
            client->sendToClient(":" + server->getHostname() + " 403 " + nickOrStar(client) + " " + target + " :No such channel");
            return;
        }

        if (params.size() == 1) {
            std::string modes = "+";
            std::string extra;

            if (channel->isInviteOnly()) {
                modes += "i";
            }
            if (channel->isTopicRestricted()) {
                modes += "t";
            }
            if (!channel->getKey().empty()) {
                modes += "k";
                extra += " " + channel->getKey();
            }
            if (channel->getUserLimit() > 0) {
                std::ostringstream oss;
                oss << channel->getUserLimit();
                modes += "l";
                extra += " " + oss.str();
            }

            client->sendToClient(":" + server->getHostname() + " 324 " +
                client->getNickname() + " " + target + " " + modes + extra);
            return;
        }

        if (!channel->isOperator(client)) {
            client->sendToClient(":" + server->getHostname() + " 482 " + nickOrStar(client) + " " + target + " :You're not channel operator");
            return;
        }

        std::string modeStr = params[1];
        bool adding = true;
        size_t paramIndex = 2;

        for (size_t i = 0; i < modeStr.size(); ++i) {
            char c = modeStr[i];
            if (c == '+') {
                adding = true;
            } else if (c == '-') {
                adding = false;
            } else if (c == 'i') {
                channel->setInviteOnly(adding);
                server->broadcastToChannel(channel, ":" + client->getPrefix() + " MODE " + target + " " + (adding ? "+" : "-") + "i", NULL);
            } else if (c == 't') {
                channel->setTopicRestricted(adding);
                server->broadcastToChannel(channel, ":" + client->getPrefix() + " MODE " + target + " " + (adding ? "+" : "-") + "t", NULL);
            } else if (c == 'k') {
                if (adding) {
                    if (paramIndex >= params.size()) {
                        client->sendToClient(":" + server->getHostname() + " 461 " + nickOrStar(client) + " MODE :Not enough parameters");
                        continue;
                    }

                    std::string key = params[paramIndex++];
                    channel->setKey(key);
                    server->broadcastToChannel(channel,
                        ":" + client->getPrefix() + " MODE " + target + " +k " + key,
                        NULL);
                } else {
                    channel->setKey("");
                    server->broadcastToChannel(channel,
                        ":" + client->getPrefix() + " MODE " + target + " -k",
                        NULL);
                }
            } else if (c == 'l') {
                if (adding) {
                    if (paramIndex >= params.size()) {
                        client->sendToClient(":" + server->getHostname() + " 461 " + nickOrStar(client) + " MODE :Not enough parameters");
                        continue;
                    }

                    std::string limitStr = params[paramIndex++];
                    if (!Utils::isPositiveNumber(limitStr) || Utils::atoi(limitStr) <= 0) {
                        client->sendToClient(":" + server->getHostname() + " 461 " + nickOrStar(client) + " MODE :Invalid limit");
                        continue;
                    }

                    unsigned int limit = static_cast<unsigned int>(Utils::atoi(limitStr));
                    channel->setUserLimit(limit);
                    server->broadcastToChannel(channel,
                        ":" + client->getPrefix() + " MODE " + target + " +l " + limitStr,
                        NULL);
                } else {
                    channel->setUserLimit(0);
                    server->broadcastToChannel(channel,
                        ":" + client->getPrefix() + " MODE " + target + " -l",
                        NULL);
                }
            } else if (c == 'o') {
                if (paramIndex >= params.size()) {
                    client->sendToClient(":" + server->getHostname() + " 461 " + nickOrStar(client) + " MODE :Not enough parameters");
                    continue;
                }
                Client* targetClient = server->getClientByNickname(params[paramIndex++]);
                if (!targetClient || !channel->isMember(targetClient)) {
                    client->sendToClient(":" + server->getHostname() + " 441 " + nickOrStar(client) + " " + target + " " + params[paramIndex - 1] + " :They aren't on that channel");
                    continue;
                }
                if (adding) {
                    if (channel->isOperator(targetClient)) {
                        continue;
                    }
                    channel->addOperator(targetClient);
                } else {
                    channel->removeOperator(targetClient);
                }
                server->broadcastToChannel(channel, ":" + client->getPrefix() + " MODE " + target + " " + (adding ? "+" : "-") + "o " + targetClient->getNickname(), NULL);
            } else {
                client->sendToClient(":" + server->getHostname() + " 472 " + client->getNickname() + " " + std::string(1, c) + " :is unknown mode char to me");
            }
        }
    } else {
        if (target != client->getNickname()) {
            client->sendToClient(":" + server->getHostname() + " 502 " +
                client->getNickname() + " :Cannot change mode for other users");
            return;
        }

        if (params.size() == 1) {
            client->sendToClient(":" + server->getHostname() + " 221 " +
                client->getNickname() + " +");
            return;
        }

        client->sendToClient(":" + server->getHostname() + " 501 " +
            client->getNickname() + " :Unknown MODE flag");
        return;
    }
}

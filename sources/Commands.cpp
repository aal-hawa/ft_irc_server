#include "../includes/Commands.hpp"
#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/Message.hpp"
#include "../includes/Utils.hpp"
#include <sstream>
#include <set>

static void tryRegister(Server* server, Client*client)
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

//PING
void Command_PING(Server* server, Client* client, const Message& message)
{
    (void)server;
    std::vector<std::string> params = message.getParams();
    std::string token;
    if(!params.empty())
        token = params[0];
    client-> sendToClient(":ircserv PONG :" + token);
}

//CAP 
void Command_CAP(Server* server, Client* client, const Message& message)
{
    (void)server;
    std::vector<std::string> params = message.getParams();
    if (params.empty()) return;

     if (!params.empty() && params[0] == "LS")
    {
        client->sendToClient(":ircserv CAP * LS :");
    }
}

// PASS Command
void Command_PASS(Server* server, Client* client, const Message& message) {
    // (void)server; // Unused parameter

    std::vector<std::string> params = message.getParams();

    if (params.empty()) {
        client->sendToClient("461 * PASS :Not enough parameters");
        return;
    }

    if (client->isAuthenticated()) {
        client->sendToClient("462 * :You may not reregister");
        return;
    }

    if (params[0] == server->getPassword()) {
        client->setAuthenticated(true);
        tryRegister(server, client);
    } else {
        client->sendToClient("464 * :Password incorrect");
        client->setAuthenticated(false);
    }
}

// NICK Command
void Command_NICK(Server* server, Client* client, const Message& message)
{
    std::vector<std::string> params = message.getParams();

    if (params.empty()) {
        client->sendToClient("431 * :No nickname given");
        return;
    }

    std::string newNick = params[0];

    if (!Utils::isValidNickname(newNick)) {
        client->sendToClient("432 " + newNick + " :Erroneus nickname");
        return;
    }

    Client* nickOwner = server->getClientByNickname(newNick);
    if (nickOwner && nickOwner != client) {
        client->sendToClient(":" + server->getHostname() + " 433 " + newNick + " :Nickname is already in use");
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

// USER Command
void Command_USER(Server* server, Client* client, const Message& message) {
    std::vector<std::string> params = message.getParams();

    if (params.size() < 4) {
        client->sendToClient("461 * USER :Not enough parameters");
        return;
    }

    if (client->isRegistered()) {
        client->sendToClient("462 * :You may not reregister");
        return;
    }

    if (!Utils::isValidUsername(params[0])) {
        client->sendToClient(":" + server->getHostname() + " 461 * USER :Invalid username");
        return;
    }

    client->setUsername(params[0]);
    client->setRealname(params[3]);

    tryRegister(server, client);
}

// JOIN Command
void Command_JOIN(Server* server, Client* client, const Message& message)
{
    std::vector<std::string> params = message.getParams();

    if (params.empty()) {
        client->sendToClient("461 * JOIN :Not enough parameters");
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
            client->sendToClient("403 " + channelName + " :No such channel");
            continue;
        }

        Channel* channel = server->getChannel(channelName);
        if (!channel) {
            channel = new Channel(channelName, client);
            server->addChannel(channel);
        } else {
            if (channel->isMember(client)) {
                continue;
            }

            if (channel->isInviteOnly() && !channel->isInvited(client->getNickname())) {
                client->sendToClient("473 " + channelName + " :Cannot join channel (+i)");
                continue;
            }

            if (!channel->getKey().empty() && channel->getKey() != key) {
                client->sendToClient("475 " + channelName + " :Cannot join channel (+k)");
                continue;
            }

            if (channel->getUserLimit() > 0 && channel->getMemberCount() >= channel->getUserLimit()) {
                client->sendToClient("471 " + channelName + " :Cannot join channel (+l)");
                continue;
            }
        }

        channel->addMember(client);
        channel->uninviteUser(client->getNickname());

        if (channel->getMemberCount() == 1) {
            channel->addOperator(client);
        }

        client->sendToClient(":" + client->getPrefix() + " JOIN :" + channelName);

       if (!channel->getTopic().empty()) {
            client->sendToClient(":" + server->getHostname() + " 332 " +
                client->getNickname() + " " + channelName + " :" + channel->getTopic());
        } else {
            client->sendToClient(":" + server->getHostname() + " 331 " +
                client->getNickname() + " " + channelName + " :No topic is set");
        }

        server->sendNames(client, channel);
        server->broadcastToChannel(channel, ":" + client->getPrefix() + " JOIN :" + channelName, client);
    }
}

// PART Command
void Command_PART(Server* server, Client* client, const Message& message) {
    std::vector<std::string> params = message.getParams();

    if (params.empty()) {
        client->sendToClient("461 * PART :Not enough parameters");
        return;
    }

    std::vector<std::string> channels = Utils::split(params[0], ',');
    std::string reason = (params.size() > 1) ? params[1] : client->getNickname();

    for (size_t i = 0; i < channels.size(); ++i) {
        std::string channelName = channels[i];
        Channel* channel = server->getChannel(channelName);

        if (!channel) {
            client->sendToClient("403 " + channelName + " :No such channel");
            continue;
        }

        if (!channel->isMember(client)) {
            client->sendToClient("442 " + channelName + " :You're not on that channel");
            continue;
        }

        std::string partMsg = ":" + client->getPrefix() + " PART " + channelName + " :" + reason;
        server->broadcastToChannel(channel, partMsg, NULL);
        channel->removeMember(client);

        if (channel->getMemberCount() == 0) {
            server->removeChannel(channel);
            delete channel;
        }
    }
}

// PRIVMSG Command
void Command_PRIVMSG(Server* server, Client* client, const Message& message) {
    std::vector<std::string> params = message.getParams();

    if (params.empty()) {
        client->sendToClient(":" + server->getHostname() + " 411 * :No recipient given (PRIVMSG)");
        return;
    }

    if (params.size() < 2) {
        client->sendToClient(":" + server->getHostname() + " 412 * :No text to send");
        return;
    }

    std::string target = params[0];
    std::string text = params[1];

    if (target.empty()) {
        client->sendToClient(":" + server->getHostname() + " 411 * :No recipient given (PRIVMSG)");
        return;
    }

    if (target[0] == '#' || target[0] == '&') {
        Channel* channel = server->getChannel(target);
        if (!channel) {
            client->sendToClient("403 " + target + " :No such channel");
            return;
        }

        if (!channel->isMember(client)) {
            client->sendToClient("404 " + target + " :Cannot send to channel");
            return;
        }

        std::string msg = ":" + client->getPrefix() + " PRIVMSG " + target + " :" + text;
        server->broadcastToChannel(channel, msg, client);
    } else {
        Client* targetClient = server->getClientByNickname(target);
        if (!targetClient) {
            client->sendToClient("401 " + target + " :No such nick/channel");
            return;
        }

        std::string msg = ":" + client->getPrefix() + " PRIVMSG " + target + " :" + text;
        targetClient->sendToClient(msg);
    }
}

// QUIT Command
void Command_QUIT(Server* server, Client* client, const Message& message) {
    std::string reason;
    std::vector<std::string> params = message.getParams();
    if (!params.empty()) {
        reason = params[0];
    } else {
        reason = client->getNickname();
    }

    std::string quitMsg = ":" + client->getPrefix() + " QUIT :" + reason;

    // Remove from all channels
    std::vector<Channel*> channels = server->getChannelsByClient(client);
    for (size_t i = 0; i < channels.size(); ++i) {
        server->broadcastToChannel(channels[i], quitMsg, client);
        channels[i]->removeMember(client);
        if (channels[i]->getMemberCount() == 0) {
            server->removeChannel(channels[i]);
            delete channels[i];
        }
    }

    // Remove from server
    server->removeClient(client->getFd());
    // delete client;
}

// KICK Command
void Command_KICK(Server* server, Client* client, const Message& message) {
    std::vector<std::string> params = message.getParams();

    if (params.size() < 2) {
        client->sendToClient("461 * KICK :Not enough parameters");
        return;
    }

    std::string channelName = params[0];
    std::string nickname = params[1];
    std::string comment = (params.size() > 2) ? params[2] : client->getNickname();

    Channel* channel = server->getChannel(channelName);
    if (!channel) {
        client->sendToClient("403 " + channelName + " :No such channel");
        return;
    }

    if (!channel->isOperator(client)) {
        client->sendToClient("482 " + channelName + " :You're not channel operator");
        return;
    }

    Client* targetClient = server->getClientByNickname(nickname);
    if (!targetClient || !channel->isMember(targetClient)) {
        client->sendToClient("441 " + channelName + " " + nickname + " :They aren't on that channel");
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

// INVITE Command
void Command_INVITE(Server* server, Client* client, const Message& message) {
    std::vector<std::string> params = message.getParams();

    if (params.size() < 2) {
        client->sendToClient("461 * INVITE :Not enough parameters");
        return;
    }

    std::string nickname = params[0];
    std::string channelName = params[1];

    Channel* channel = server->getChannel(channelName);
    if (!channel) {
        client->sendToClient("403 " + channelName + " :No such channel");
        return;
    }

    if (!channel->isMember(client)) {
        client->sendToClient("442 " + channelName + " :You're not on that channel");
        return;
    }

   if (!channel->isOperator(client))
    {
        client->sendToClient("482 " + channelName + " :You're not channel operator");
        return;
    }

    Client* targetClient = server->getClientByNickname(nickname);
    if (!targetClient) {
        client->sendToClient("401 " + nickname + " :No such nick");
        return;
    }

    if (channel->isMember(targetClient)) {
        client->sendToClient("443 " + nickname + " " + channelName + " :is already on channel");
        return;
    }

    channel->inviteUser(nickname);
    client->sendToClient("341 " + client->getNickname() + " " + nickname + " " + channelName);
    targetClient->sendToClient(":" + client->getPrefix() + " INVITE " + nickname + " :" + channelName);
}

// TOPIC Command
void Command_TOPIC(Server* server, Client* client, const Message& message) {
    std::vector<std::string> params = message.getParams();

    if (params.empty()) {
        client->sendToClient("461 * TOPIC :Not enough parameters");
        return;
    }

    std::string channelName = params[0];
    Channel* channel = server->getChannel(channelName);

    if (!channel) {
        client->sendToClient("403 " + channelName + " :No such channel");
        return;
    }

    if (!channel->isMember(client)) {
        client->sendToClient("442 " + channelName + " :You're not on that channel");
        return;
    }

    if (params.size() == 1) {
        if (channel->getTopic().empty()) {
            client->sendToClient(":" + server->getHostname() + " 331 " + client->getNickname() + " " + channelName + " :No topic is set");
        } else {
            client->sendToClient(":" + server->getHostname() + " 332 " + client->getNickname() + " " + channelName + " :" + channel->getTopic());
        }
        return;
    }

    if (channel->isTopicRestricted() && !channel->isOperator(client)) {
        client->sendToClient("482 " + channelName + " :You're not channel operator");
        return;
    }

    std::string newTopic = params[1];
    channel->setTopic(newTopic);
    server->broadcastToChannel(channel, ":" + client->getPrefix() + " TOPIC " + channelName + " :" + newTopic, NULL);
}

// MODE Command
void Command_MODE(Server* server, Client* client, const Message& message)
{
    std::vector<std::string> params = message.getParams();

    if (params.empty()) {
        client->sendToClient("461 * MODE :Not enough parameters");
        return;
    }

    std::string target = params[0];

    // Channel mode
    if (target[0] == '#' || target[0] == '&') {
        Channel* channel = server->getChannel(target);
        if (!channel) {
            client->sendToClient("403 " + target + " :No such channel");
            return;
        }

        if (params.size() == 1) {
            // View current modes
            std::string modes = "+";
            if (channel->isInviteOnly()) modes += "i";
            if (channel->isTopicRestricted()) modes += "t";
            if (!channel->getKey().empty()) modes += "k";
            if (channel->getUserLimit() > 0) modes += "l";
            client->sendToClient(":" + server->getHostname() + " 324 " + client->getNickname() + " " + target + " " + modes);
            return;
        }

        // Change modes
        if (!channel->isOperator(client)) {
            client->sendToClient("482 " + target + " :You're not channel operator");
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
                        client->sendToClient("461 " + target + " MODE :Not enough parameters");
                        continue;
                    }
                    channel->setKey(params[paramIndex++]);
                } else {
                    channel->setKey("");
                }
server->broadcastToChannel(channel, ":" + client->getPrefix() + " MODE " + target + " " + (adding ? "+" : "-") + "k", NULL);            } else if (c == 'l') {
                if (adding) {
                    if (paramIndex >= params.size()) {
                        client->sendToClient("461 " + target + " MODE :Not enough parameters");
                        continue;
                    }
                std::string limitStr = params[paramIndex++];
                if (!Utils::isPositiveNumber(limitStr) || Utils::atoi(limitStr) <= 0)
                {
                    client->sendToClient(":" + server->getHostname() + " 461 " + target + " MODE :Invalid limit");
                    continue;
                }
                    unsigned int limit = static_cast <unsigned int>(Utils::atoi(limitStr));
                    channel->setUserLimit(limit);
                } else {
                    channel->setUserLimit(0);
                }
                server->broadcastToChannel(channel, ":" + client->getPrefix() + " MODE " + target + " " + (adding ? "+" : "-") + "l", NULL);
            } else if (c == 'o') {
                if (paramIndex >= params.size()) {
                    client->sendToClient("461 " + target + " MODE :Not enough parameters");
                    continue;
                }
                Client* targetClient = server->getClientByNickname(params[paramIndex++]);
                if (!targetClient || !channel->isMember(targetClient)) {
                    client->sendToClient("441 " + target + " " + params[paramIndex-1] + " :They aren't on that channel");
                    continue;
                }
                if (adding) {
                    channel->addOperator(targetClient);
                } else {
                    channel->removeOperator(targetClient);
                }
               server->broadcastToChannel(channel, ":" + client->getPrefix() + " MODE " + target + " " + (adding ? "+" : "-") + "o " + targetClient->getNickname(), NULL);
            }
        }
    }
}

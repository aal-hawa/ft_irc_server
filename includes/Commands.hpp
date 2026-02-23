#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "Server.hpp"
#include "Client.hpp"
#include "Message.hpp"

class Server;
class Client;
class Message;

// Command functions
void Command_PASS(Server* server, Client* client, const Message& message);
void Command_NICK(Server* server, Client* client, const Message& message);
void Command_USER(Server* server, Client* client, const Message& message);
void Command_JOIN(Server* server, Client* client, const Message& message);
void Command_PART(Server* server, Client* client, const Message& message);
void Command_PRIVMSG(Server* server, Client* client, const Message& message);
void Command_QUIT(Server* server, Client* client, const Message& message);
void Command_KICK(Server* server, Client* client, const Message& message);
void Command_INVITE(Server* server, Client* client, const Message& message);
void Command_TOPIC(Server* server, Client* client, const Message& message);
void Command_MODE(Server* server, Client* client, const Message& message);
void Command_CAP(Server* server, Client* client, const Message& message);
void Command_PING(Server* server, Client* client, const Message& message);

#endif

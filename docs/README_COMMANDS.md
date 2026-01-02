# Commands Module Documentation

## Overview

The `Commands` module implements all IRC protocol commands that the server supports. This module contains command handler functions that parse client messages, validate permissions, and execute the corresponding actions. Each command follows the IRC protocol specifications (RFC 1459 and RFC 2812).

## Purpose

The Commands module serves as the command processing layer of the IRC server:
- Parses and validates IRC commands from clients
- Enforces protocol rules and permissions
- Updates server and channel state
- Sends appropriate responses to clients
- Handles error conditions with proper IRC error codes

## Command Functions

The module implements the following IRC commands:
1. **PASS** - Password authentication
2. **NICK** - Set/change nickname
3. **USER** - Set user information
4. **JOIN** - Join a channel
5. **PART** - Leave a channel
6. **PRIVMSG** - Send messages (private or channel)
7. **QUIT** - Disconnect from server
8. **KICK** - Remove user from channel
9. **INVITE** - Invite user to channel
10. **TOPIC** - View/set channel topic
11. **MODE** - Change channel/user modes

---

## PASS Command

### Function Signature
```cpp
void Command_PASS(Server* server, Client* client, const Message& message);
```

### Purpose
Authenticates a client by verifying the server password.

### Parameters
- `server` - Pointer to the Server instance
- `client` - Pointer to the Client sending the command
- `message` - The parsed Message object containing command parameters

### Behavior
1. Validates that password parameter is provided (error 461 if missing)
2. Checks if already authenticated (error 462 if already authenticated)
3. Compares provided password with server password
4. Sets client authentication status accordingly
5. Sends error 464 if password is incorrect

### IRC Responses
- **461**: `* PASS :Not enough parameters`
- **462**: `* :You may not reregister`
- **464**: `* :Password incorrect`

### Usage Example
```
PASS mypassword
```

---

## NICK Command

### Function Signature
```cpp
void Command_NICK(Server* server, Client* client, const Message& message);
```

### Purpose
Sets or changes a client's nickname.

### Parameters
- `server` - Pointer to the Server instance
- `client` - Pointer to the Client sending the command
- `message` - The parsed Message object

### Behavior
1. Validates that nickname parameter is provided (error 431 if missing)
2. Validates nickname format using `Utils::isValidNickname()` (error 432 if invalid)
3. Checks if nickname is already in use (error 433 if taken)
4. Updates client's nickname
5. Sends NICK change notification to all channels if already registered
6. Triggers registration if both NICK and USER are set
7. Sends welcome messages to newly registered clients

### Validation Rules
- Maximum 9 characters
- First character: letter or [ \ ] ^ _ `
- Subsequent characters: letter, digit, or - [ \ ] ^ _ `

### IRC Responses
- **431**: `* :No nickname given`
- **432**: `* :Erroneus nickname`
- **433**: `* :Nickname is already in use`
- **NICK change notification**: `:oldnick NICK :newnick`

### Usage Example
```
NICK alice
```

---

## USER Command

### Function Signature
```cpp
void Command_USER(Server* server, Client* client, const Message& message);
```

### Purpose
Sets the client's username and real name (part of registration process).

### Parameters
- `server` - Pointer to the Server instance
- `client` - Pointer to the Client sending the command
- `message` - The parsed Message object

### Behavior
1. Validates that all 4 parameters are provided (error 461 if missing)
2. Checks if already registered (error 462 if already registered)
3. Sets client's username (parameter 0) and real name (parameter 3)
4. Triggers registration if both NICK and USER are set and authenticated
5. Sends welcome messages to newly registered clients

### Parameters
- `username` - The client's username (max 9 characters)
- `mode` - User mode (ignored in this implementation)
- `unused` - Unused parameter (ignored)
- `realname` - The client's real name

### IRC Responses
- **461**: `* USER :Not enough parameters`
- **462**: `* :You may not reregister`

### Usage Example
```
USER username 0 * :Real Name Here
```

---

## JOIN Command

### Function Signature
```cpp
void Command_JOIN(Server* server, Client* client, const Message& message);
```

### Purpose
Joins one or more IRC channels.

### Parameters
- `server` - Pointer to the Server instance
- `client` - Pointer to the Client sending the command
- `message` - The parsed Message object

### Behavior
1. Validates that channel list is provided (error 461 if missing)
2. Parses comma-separated channel list and key list
3. For each channel:
   - Validates channel name format (error 403 if invalid)
   - Creates new channel if it doesn't exist
   - Checks invite-only restriction (error 473 if needed)
   - Checks password requirement (error 475 if wrong/no key)
   - Checks user limit (error 471 if full)
   - Adds client to channel
   - Makes client operator if first member
   - Sends JOIN notification to channel
   - Sends topic to client if set
   - Sends names list to client

### Parameters
- `channels` - Comma-separated list of channels to join
- `keys` - Comma-separated list of passwords (optional)

### IRC Responses
- **461**: `* JOIN :Not enough parameters`
- **403**: `channel :No such channel`
- **473**: `channel :Cannot join channel (+i)`
- **475**: `channel :Cannot join channel (+k)`
- **471**: `channel :Cannot join channel (+l)`
- **332**: `channel :topic` (topic message)
- **353**: `= channel :nicklist` (names list)
- **366**: `channel :End of /NAMES list`
- **JOIN notification**: `:prefix JOIN :channel`

### Usage Examples
```
JOIN #general
JOIN #channel1,#channel2 key1,key2
```

---

## PART Command

### Function Signature
```cpp
void Command_PART(Server* server, Client* client, const Message& message);
```

### Purpose
Leaves one or more IRC channels.

### Parameters
- `server` - Pointer to the Server instance
- `client` - Pointer to the Client sending the command
- `message` - The parsed Message object

### Behavior
1. Validates that channel list is provided (error 461 if missing)
2. Parses comma-separated channel list
3. For each channel:
   - Verifies channel exists (error 403 if not)
   - Verifies client is member (error 442 if not)
   - Broadcasts PART message to channel
   - Removes client from channel
   - Deletes channel if empty

### Parameters
- `channels` - Comma-separated list of channels to leave
- `reason` - Part reason (optional, defaults to nickname)

### IRC Responses
- **461**: `* PART :Not enough parameters`
- **403**: `channel :No such channel`
- **442**: `channel :You're not on that channel`
- **PART notification**: `:prefix PART channel :reason`

### Usage Examples
```
PART #general
PART #channel1,#channel2 :Leaving now
```

---

## PRIVMSG Command

### Function Signature
```cpp
void Command_PRIVMSG(Server* server, Client* client, const Message& message);
```

### Purpose
Sends a private message to a user or a channel.

### Parameters
- `server` - Pointer to the Server instance
- `client` - Pointer to the Client sending the command
- `message` - The parsed Message object

### Behavior
1. Validates that target and message text are provided (errors 411, 412)
2. Checks if target is a channel (starts with # or &)
3. For channel messages:
   - Verifies channel exists (error 403 if not)
   - Verifies client is member (error 404 if not)
   - Broadcasts message to all channel members except sender
4. For private messages:
   - Verifies target user exists (error 401 if not)
   - Sends message directly to target user

### Parameters
- `target` - Channel name or user nickname
- `text` - The message text to send

### IRC Responses
- **411**: `* :No recipient given (PRIVMSG)`
- **412**: `* :No text to send`
- **403**: `channel :No such channel`
- **404**: `channel :Cannot send to channel`
- **401**: `target :No such nick/channel`
- **Message**: `:prefix PRIVMSG target :text`

### Usage Examples
```
PRIVMSG #general :Hello everyone!
PRIVMSG alice :Private message for you
```

---

## QUIT Command

### Function Signature
```cpp
void Command_QUIT(Server* server, Client* client, const Message& message);
```

### Purpose
Disconnects a client from the server.

### Parameters
- `server` - Pointer to the Server instance
- `client` - Pointer to the Client disconnecting
- `message` - The parsed Message object

### Behavior
1. Extracts quit reason (defaults to nickname)
2. Broadcasts QUIT message to all channels the client is in
3. Removes client from all channels
4. Deletes empty channels
5. Removes client from server
6. Deletes client object

### Parameters
- `reason` - Quit reason (optional)

### IRC Responses
- **QUIT notification**: `:prefix QUIT :reason`

### Usage Examples
```
QUIT
QUIT :Leaving now
```

---

## KICK Command

### Function Signature
```cpp
void Command_KICK(Server* server, Client* client, const Message& message);
```

### Purpose
Removes a user from a channel (operator only).

### Parameters
- `server` - Pointer to the Server instance
- `client` - Pointer to the Client executing the command
- `message` - The parsed Message object

### Behavior
1. Validates parameters (error 461 if missing)
2. Verifies channel exists (error 403 if not)
3. Verifies sender is channel operator (error 482 if not)
4. Verifies target user exists and is member (error 441 if not)
5. Broadcasts KICK message to channel
6. Removes target user from channel
7. Deletes channel if empty

### Parameters
- `channel` - Channel to kick from
- `nickname` - User to kick
- `comment` - Kick reason (optional, defaults to operator's nickname)

### IRC Responses
- **461**: `* KICK :Not enough parameters`
- **403**: `channel :No such channel`
- **482**: `channel :You're not channel operator`
- **441**: `channel nickname :They aren't on that channel`
- **KICK notification**: `:prefix KICK channel nickname :comment`

### Usage Examples
```
KICK #general spammer :No spamming allowed
```

---

## INVITE Command

### Function Signature
```cpp
void Command_INVITE(Server* server, Client* client, const Message& message);
```

### Purpose
Invites a user to a channel (bypasses invite-only restriction).

### Parameters
- `server` - Pointer to the Server instance
- `client` - Pointer to the Client sending the invitation
- `message` - The parsed Message object

### Behavior
1. Validates parameters (error 461 if missing)
2. Verifies channel exists (error 403 if not)
3. Verifies sender is channel member (error 442 if not)
4. Verifies sender is operator if channel is invite-only (error 482 if not)
5. Verifies target user exists (error 401 if not)
6. Verifies target is not already member (error 443 if member)
7. Adds target to channel's invite list
8. Sends invitation confirmation to sender
9. Sends INVITE message to target

### Parameters
- `nickname` - User to invite
- `channel` - Channel to invite to

### IRC Responses
- **461**: `* INVITE :Not enough parameters`
- **403**: `channel :No such channel`
- **442**: `channel :You're not on that channel`
- **482**: `channel :You're not channel operator`
- **401**: `nickname :No such nick`
- **443**: `nickname channel :is already on channel`
- **341**: `sender nickname channel` (invitation confirmation)
- **INVITE message**: `:prefix INVITE nickname :channel`

### Usage Example
```
INVITE alice #general
```

---

## TOPIC Command

### Function Signature
```cpp
void Command_TOPIC(Server* server, Client* client, const Message& message);
```

### Purpose
Views or changes a channel's topic.

### Parameters
- `server` - Pointer to the Server instance
- `client` - Pointer to the Client executing the command
- `message` - The parsed Message object

### Behavior

**Viewing topic (no topic parameter)**:
1. Verifies channel exists (error 403 if not)
2. Verifies sender is member (error 442 if not)
3. Sends current topic (error 331 if not set, RPL_TOPIC if set)

**Setting topic (with topic parameter)**:
1. Verifies channel exists (error 403 if not)
2. Verifies sender is member (error 442 if not)
3. Verifies sender is operator if topic restricted (error 482 if not)
4. Updates channel topic
5. Broadcasts topic change to channel

### Parameters
- `channel` - Channel name
- `topic` - New topic (optional)

### IRC Responses
- **461**: `* TOPIC :Not enough parameters`
- **403**: `channel :No such channel`
- **442**: `channel :You're not on that channel`
- **331**: `channel :No topic is set`
- **332**: `channel :topic` (topic message)
- **482**: `channel :You're not channel operator`
- **TOPIC change**: `:server TOPIC channel :newtopic`

### Usage Examples
```
TOPIC #general
TOPIC #general :New topic here
```

---

## MODE Command

### Function Signature
```cpp
void Command_MODE(Server* server, Client* client, const Message& message);
```

### Purpose
Views or changes channel modes and user modes (operators only).

### Parameters
- `server` - Pointer to the Server instance
- `client` - Pointer to the Client executing the command
- `message` - The parsed Message object

### Behavior

**Viewing modes (no mode string)**:
1. Verifies channel exists (error 403 if not)
2. Sends current channel modes (RPL_CHANNELMODEIS)

**Changing modes**:
1. Verifies channel exists (error 403 if not)
2. Verifies sender is channel operator (error 482 if not)
3. Parses mode string (+i, -i, +k, -k, +l, -l, +t, -t, +o, -o)
4. Applies each mode change
5. Broadcasts mode changes to channel

### Supported Channel Modes

| Mode | Type | Parameter | Description |
|------|------|-----------|-------------|
| +i | Invite-only | None | Users can only join if invited |
| -i | Invite-only | None | Anyone can join |
| +k | Password | key | Requires password to join |
| -k | Password | None | Removes password requirement |
| +l | User limit | number | Sets maximum users |
| -l | User limit | None | Removes user limit |
| +t | Topic restricted | None | Only operators can change topic |
| -t | Topic restricted | None | Anyone can change topic |
| +o | Operator | nickname | Grants operator privileges |
| -o | Operator | nickname | Removes operator privileges |

### Parameters
- `target` - Channel name
- `modes` - Mode string (e.g., "+ikl", "-i")
- Additional parameters for +k (key), +l (number), +o/-o (nickname)

### IRC Responses
- **461**: `* MODE :Not enough parameters`
- **403**: `channel :No such channel`
- **482**: `channel :You're not channel operator`
- **324**: `channel +modes` (current modes)
- **MODE change**: `:server MODE channel +modes`

### Usage Examples
```
MODE #general
MODE #general +i
MODE #general +k secretpass
MODE #general +l 50
MODE #general +o alice
MODE #general -i-k+l
```

---

## Error Codes Summary

| Code | Name | Description |
|------|------|-------------|
| 331 | RPL_NOTOPIC | No topic is set |
| 332 | RPL_TOPIC | Channel topic |
| 341 | RPL_INVITING | Invitation sent |
| 353 | RPL_NAMREPLY | Names list |
| 366 | RPL_ENDOFNAMES | End of names list |
| 401 | ERR_NOSUCHNICK | No such nick/channel |
| 403 | ERR_NOSUCHCHANNEL | No such channel |
| 404 | ERR_CANNOTSENDTOCHAN | Cannot send to channel |
| 411 | ERR_NORECIPIENT | No recipient given |
| 412 | ERR_NOTEXTTOSEND | No text to send |
| 421 | ERR_UNKNOWNCOMMAND | Unknown command |
| 431 | ERR_NONICKNAMEGIVEN | No nickname given |
| 432 | ERR_ERRONEUSNICKNAME | Erroneous nickname |
| 433 | ERR_NICKNAMEINUSE | Nickname already in use |
| 441 | ERR_USERNOTINCHANNEL | User not in channel |
| 442 | ERR_NOTONCHANNEL | Not on that channel |
| 443 | ERR_USERONCHANNEL | User already on channel |
| 461 | ERR_NEEDMOREPARAMS | Not enough parameters |
| 462 | ERR_ALREADYREGISTRED | Already registered |
| 464 | ERR_PASSWDMISMATCH | Password incorrect |
| 471 | ERR_CHANNELISFULL | Channel is full |
| 473 | ERR_INVITEONLYCHAN | Cannot join (+i) |
| 475 | ERR_BADCHANNELKEY | Wrong channel password |
| 482 | ERR_CHANOPRIVSNEEDED | Not channel operator |

## Dependencies

- `Server.hpp` - Server class
- `Client.hpp` - Client class
- `Message.hpp` - Message class
- `Utils.hpp` - Validation and utility functions
- `sstream` - String stream for parsing

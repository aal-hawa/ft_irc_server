*This project has been created as part of the 42 curriculum by aal-hawa.*

# ft_irc - IRC Server Implementation
A complete IRC (Internet Relay Chat) server implementation in C++ 98 for the 42 School project.

---

## Features

- **Multi-client support** using non-blocking I/O and `poll()`
- **Non-blocking operations only** - no forking
- **Single `poll()` call** for all I/O operations (POLLIN and POLLOUT)
- **Buffered sending** - messages are only sent when `poll()` indicates POLLOUT is ready
- **Both `\r\n` and `\n` support** - compatible with telnet and nc (netcat)
- **Basic IRC commands**: PASS, NICK, USER, JOIN, PART, PRIVMSG, QUIT
- **Channel operations**: JOIN, PART, KICK, INVITE, TOPIC, MODE
- **Channel modes**: `i` (invite-only), `t` (topic restricted), `k` (key), `o` (operator), `l` (limit)
- **Keepalive support**: PING/PONG commands
- **Proper error handling** with RFC 2812 compliant responses
- **Partial message handling** to support fragmented data
- **Memory leak-free** implementation

---

## Requirements

- **Language**: C++ 98 standard
- **Compiler**: clang++ or g++
- **Operating System**: Unix-based (Linux, macOS)
- **External Libraries**: None (only standard socket functions)

---

## Project Structure

```
ft_irc_server/
├── includes/          # Header files
│   ├── Server.hpp     # Main server class
│   ├── Client.hpp     # Client class
│   ├── Channel.hpp    # Channel class
│   ├── Message.hpp    # Message parser
│   ├── Commands.hpp   # Command declarations
│   └── Utils.hpp      # Utility functions
├── sources/           # Source files
│   ├── main.cpp       # Entry point
│   ├── Server.cpp     # Server implementation
│   ├── Client.cpp     # Client implementation
│   ├── Channel.cpp    # Channel implementation
│   ├── Message.cpp    # Message parser implementation
│   ├── Commands.cpp   # Command implementations
│   └── Utils.cpp      # Utility functions implementation
├── scripts/           # Test scripts
│   ├── test_basic_commands.sh
│   ├── test_channel_operations.sh
│   ├── test_operator_commands.sh
│   ├── test_stress.sh
│   └── final_test.sh
├── Makefile           # Build configuration
└── README.md          # This file
```

---

## Building

```bash
# Clean build
make clean

# Compile the project
make

# Force rebuild
make re
```

---

## Usage

```bash
./ircserv <port> <password>
```

**Example:**
```bash
./ircserv 6667 mypassword
```

---

## Connecting to the Server

### Using netcat (nc):
```bash
nc -C localhost 6667
PASS mypassword
NICK mynickname
USER myusername 0 * :My Real Name
JOIN #testchannel
PRIVMSG #testchannel :Hello everyone!
```

### Using telnet:
```bash
telnet localhost 6667
PASS mypassword
NICK mynickname
USER myusername 0 * :My Real Name
JOIN #testchannel
PRIVMSG #testchannel :Hello everyone!
```

### Using IRC client (irssi):
```bash
irssi -c localhost -p 6667 -w mypassword
```

### Using IRC client (HexChat):
- Server: `localhost`
- Port: `6667`
- Password: `mypassword`

---

## Implemented Commands

### Authentication Commands

#### PASS
Set the connection password
```
PASS <password>
```

#### NICK
Set or change your nickname
```
NICK <nickname>
```

#### USER
Set your username and realname
```
USER <username> <mode> <unused> :<realname>
```

### Channel Commands

#### JOIN
Join one or more channels
```
JOIN <channel>[,<channel>...] [<key>[,<key>...]]
```

#### PART
Leave one or more channels
```
PART <channel>[,<channel>...] [<reason>]
```

#### PRIVMSG
Send a private message to user or channel
```
PRIVMSG <target> :<message>
```

#### QUIT
Disconnect from the server
```
QUIT [<reason>]
```

### Operator Commands

#### KICK
Remove a user from a channel
```
KICK <channel> <user> [:<comment>]
```

#### INVITE
Invite a user to a channel
```
INVITE <nickname> <channel>
```

#### TOPIC
View or change the channel topic
```
TOPIC <channel> [:<topic>]
```

#### MODE
View or change channel modes
```
MODE <channel> [<modes> [<mode parameters>]]
```

**Available modes:**
- `+i` / `-i`: Invite-only channel
- `+t` / `-t`: Topic restricted to operators
- `+k <key>` / `-k`: Set/remove channel key
- `+o <nick>` / `-o <nick>`: Give/take operator privilege
- `+l <limit>` / `-l`: Set/remove user limit

### Keepalive Commands

#### PING
Check server connection
```
PING <server>
```

#### PONG
Respond to PING
```
PONG <server>
```

---

## Testing

All test scripts are located in the `scripts/` directory:

```bash
# Make all scripts executable
chmod +x scripts/*.sh

# Run basic commands test
./scripts/test_basic_commands.sh

# Run channel operations test
./scripts/test_channel_operations.sh

# Run operator commands test
./scripts/test_operator_commands.sh

# Run stress test (20 clients)
./scripts/test_stress.sh

# Run complete final test suite
./scripts/final_test.sh
```

### Partial Message Test
To test partial message handling:
```bash
nc -C localhost 6667
# Type: PASS testpass, press Ctrl+D
# Type: mypass, press Ctrl+D
# Type: word, press Enter
```

### Memory Leak Check
```bash
valgrind --leak-check=full --show-leak-kinds=all ./ircserv 6667 testpass
```

---

## Error Codes

The server implements standard IRC error codes:

| Code | Description |
|------|-------------|
| `401` | No such nick/channel |
| `403` | No such channel |
| `404` | Cannot send to channel |
| `411` | No recipient given (PRIVMSG) |
| `412` | No text to send |
| `421` | Unknown command |
| `431` | No nickname given |
| `432` | Erroneus nickname |
| `433` | Nickname already in use |
| `441` | User not in channel |
| `442` | You're not on that channel |
| `443` | User already in channel |
| `461` | Not enough parameters |
| `462` | Already registered |
| `464` | Password incorrect |
| `471` | Cannot join (channel full) |
| `473` | Cannot join (invite only) |
| `475` | Cannot join (wrong key) |
| `482` | Not channel operator |

---

## Technical Details

- **Socket Management**: Non-blocking sockets with `fcntl()`
- **Event Loop**: Single `poll()` call handles all I/O events (POLLIN and POLLOUT)
- **Buffered Sending**: Messages are queued and only sent when `poll()` indicates POLLOUT
- **Message Parsing**: RFC 2812 compliant message format
- **Connection Handling**: Each client has its own buffer for partial messages
- **Channel Management**: Dynamic channel creation and deletion
- **Operator System**: First user in channel becomes operator by default

---

## Compliance

This implementation complies with:
- **RFC 2812**: Internet Relay Chat: Client Protocol
- **42 School ft_irc project requirements**
- **C++ 98 standard**
- **Non-blocking I/O with single `poll()`**
- **Both `\r\n` (telnet) and `\n` (nc) line endings**

---

## License

This project is part of the 42 School curriculum.

---

## Authors

- **Team Member A**: Network layer, Server core, Message parsing, Basic commands
- **Team Member B**: Data structures, Channel operations, Operator commands, Testing

---

## References

- [RFC 2812 - IRC Client Protocol](https://tools.ietf.org/html/rfc2812)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [IRC Protocol Documentation](https://irc-wiki.org/IRC_Protocol)

---

**Good luck with your evaluation!**

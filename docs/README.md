# FT_IRC_SERVER - Complete Documentation

## Overview

This repository contains a complete IRC (Internet Relay Chat) server implementation written in C++. The server implements the core IRC protocol features including client authentication, channel management, operator privileges, and real-time messaging.

## Documentation Index

### Core Components

#### [Server Class Documentation](./README_SERVER.md)
Complete documentation for the Server class - the main component that handles socket management, client connections, channel management, and message routing.

**Topics Covered**:
- Architecture and design patterns
- Private member variables
- Public and private methods
- Socket initialization and management
- Event loop with poll()
- Client and channel management
- Message broadcasting
- Usage examples and best practices

---

#### [Client Class Documentation](./README_CLIENT.md)
Complete documentation for the Client class - represents connected IRC clients.

**Topics Covered**:
- Client lifecycle and state machine
- User identity management (nickname, username, realname)
- Authentication and registration states
- Message buffers for reliable communication
- Socket communication
- IRC prefix generation
- Message sending and receiving

---

#### [Channel Class Documentation](./README_CHANNEL.md)
Complete documentation for the Channel class - manages IRC channels.

**Topics Covered**:
- Channel architecture and member hierarchy
- Member management
- Operator privileges
- Channel modes (+i, +k, +l, +t, +o)
- Invitation system
- Topic management
- Usage examples for all operations

---

#### [Commands Module Documentation](./README_COMMANDS.md)
Complete documentation for all IRC commands implemented by the server.

**Commands Documented**:
- PASS - Password authentication
- NICK - Set/change nickname
- USER - Set user information
- JOIN - Join channels
- PART - Leave channels
- PRIVMSG - Send messages
- QUIT - Disconnect from server
- KICK - Remove users from channels
- INVITE - Invite users to channels
- TOPIC - View/set channel topics
- MODE - Change channel/user modes

**Includes**: Parameters, behavior, error codes, and usage examples for each command.

---

#### [Utils Class Documentation](./README_UTILS.md)
Complete documentation for utility functions used throughout the server.

**Functions Documented**:
- Validation: `isValidPassword()`, `isValidNickname()`, `isValidUsername()`, `isValidChannelName()`
- String Manipulation: `split()`, `toUpper()`, `trim()`
- Type Conversion: `atoi()`
- Time Functions: `getCurrentTime()`

**Includes**: Implementation details, examples, and RFC 1459 compliance notes.

---

#### [Message Class Documentation](./README_MESSAGE.md)
Complete documentation for the Message class - parses IRC protocol messages.

**Topics Covered**:
- IRC message format (RFC 1459)
- Parsing raw messages into components
- Extracting prefix, command, parameters, and trailing
- Complete parsing examples
- Error handling
- IRC protocol compliance

---

#### [Main Entry Point Documentation](./README_MAIN.md)
Complete documentation for main.cpp - the server's entry point.

**Topics Covered**:
- Command-line interface
- Signal handling for graceful shutdown
- Server lifecycle management
- Error handling
- Deployment and monitoring
- Security considerations

---

### Test Scripts

#### [Basic Commands Test](./README_TEST_BASIC.sh.md)
Documentation for test_basic_commands.sh - tests fundamental IRC commands.

**Tests**:
- Wrong password rejection
- Authentication and registration
- Invalid nickname rejection
- Re-registration prevention
- Welcome message verification

---

#### [Channel Operations Test](./README_TEST_CHANNEL.sh.md)
Documentation for test_channel_operations.sh - tests channel functionality with multiple clients.

**Tests**:
- Channel creation
- Multiple clients joining
- Message broadcasting
- NAMES command
- Topic management
- Channel leaving
- Named pipe communication

---

#### [Operator Commands Test](./README_TEST_OPERATOR.sh.md)
Documentation for test_operator_commands.sh - tests operator features with three clients.

**Tests**:
- Channel operator privileges
- Channel modes (+i, +k, +l, +t, +o)
- KICK command
- INVITE system
- Permission enforcement
- Mode changes and viewing

---

#### [Stress Test](./README_TEST_STRESS.sh.md)
Documentation for test_stress.sh - performance testing with concurrent clients.

**Tests**:
- Multiple concurrent connections (default: 20)
- Simultaneous authentication and registration
- Channel with many members
- Message broadcasting to many clients
- Connection cleanup

**Performance Metrics**: Commands processed, message deliveries, network activity.

---

#### [Final Test Suite](./README_FINAL_TEST.sh.md)
Documentation for final_test.sh - comprehensive test suite with build and memory leak detection.

**Test Steps**:
1. Clean build verification
2. Server startup
3. Basic commands test
4. Channel operations test
5. Operator commands test
6. Stress test
7. Partial message test (manual)
8. Memory leak check with Valgrind
9. Cleanup

---

## Quick Start

### Building the Server

```bash
# Clone repository
git clone https://github.com/aal-hawa/ft_irc_server.git
cd ft_irc_server

# Compile
make

# Clean build (recommended before testing)
make fclean && make
```

### Running the Server

```bash
# Basic usage
./ircserv <port> <password>

# Example
./ircserv 6667 mypassword
```

### Running Tests

```bash
# Run complete test suite
./scripts/final_test.sh

# Run individual tests
./scripts/test_basic_commands.sh
./scripts/test_channel_operations.sh
./scripts/test_operator_commands.sh
./scripts/test_stress.sh
```

### Connecting with Netcat

```bash
# Start netcat connection
nc -C localhost 6667

# Authenticate
PASS mypassword

# Register
NICK mynick
USER mynick 0 * :My Real Name

# Join a channel
JOIN #general

# Send a message
PRIVMSG #general :Hello everyone!
```

---

## Project Structure

```
ft_irc_server/
├── includes/
│   ├── Server.hpp       # Server class header
│   ├── Client.hpp       # Client class header
│   ├── Channel.hpp      # Channel class header
│   ├── Commands.hpp     # IRC command declarations
│   ├── Utils.hpp        # Utility functions
│   └── Message.hpp      # Message parser
├── sources/
│   ├── Server.cpp       # Server implementation
│   ├── Client.cpp       # Client implementation
│   ├── Channel.cpp      # Channel implementation
│   ├── Commands.cpp     # IRC command handlers
│   ├── Utils.cpp        # Utility implementation
│   ├── Message.cpp      # Message parser
│   └── main.cpp         # Entry point
├── scripts/
│   ├── test_basic_commands.sh
│   ├── test_channel_operations.sh
│   ├── test_operator_commands.sh
│   ├── test_stress.sh
│   └── final_test.sh
├── docs/                # This directory
│   ├── README.md        # This file
│   ├── README_SERVER.md
│   ├── README_CLIENT.md
│   ├── README_CHANNEL.md
│   ├── README_COMMANDS.md
│   ├── README_UTILS.md
│   ├── README_MESSAGE.md
│   ├── README_MAIN.md
│   ├── README_TEST_BASIC.sh.md
│   ├── README_TEST_CHANNEL.sh.md
│   ├── README_TEST_OPERATOR.sh.md
│   ├── README_TEST_STRESS.sh.md
│   └── README_FINAL_TEST.sh.md
├── Makefile             # Build configuration
└── README.md            # Main project README
```

---

## IRC Protocol Compliance

This server implements core features from:

- **RFC 1459** - Internet Relay Chat Protocol (basic)
- **RFC 2812** - Internet Relay Chat: Client Protocol (updated)

### Supported Features

✓ Client authentication (PASS)
✓ Nickname management (NICK)
✓ User registration (USER)
✓ Channel joining/leaving (JOIN/PART)
✓ Channel and private messaging (PRIVMSG)
✓ Operator privileges (MODE +o, KICK)
✓ Channel modes (+i, +k, +l, +t)
✓ Topic management (TOPIC)
✓ Invitation system (INVITE)
✓ Member lists (NAMES)
✓ Graceful shutdown (QUIT)

### Not Implemented

- Away status (AWAY)
- Server linking (server-to-server)
- User modes
- Ban lists
- WHO/WHOIS commands
- LIST command
- MOTD (Message of the Day)
- IRCv3 features

---

## Architecture

### Design Patterns

1. **Event-Driven Server**: Uses poll() for non-blocking I/O multiplexing
2. **State Machine**: Clients transition through connection states
3. **Static Utility Class**: Utils provides helper functions
4. **Parser Pattern**: Message class parses IRC protocol
5. **Command Pattern**: Each IRC command has separate handler function

### Technology Stack

- **Language**: C++ (C++98 compatible)
- **Compiler**: g++
- **Build System**: Make
- **System Calls**: socket(), bind(), listen(), accept(), poll(), recv(), send()
- **Network**: TCP/IP (IPv4)
- **Non-blocking I/O**: fcntl() with O_NONBLOCK

---

## Performance Characteristics

### Scalability

- **Concurrent Clients**: Tested with 20+ clients
- **File Descriptors**: Limited by ulimit (typically 1024)
- **Memory**: ~5-10MB per client
- **CPU**: Event-driven, minimal idle CPU

### Throughput

- **Messages/Second**: Hundreds to thousands (depends on load)
- **Latency**: Sub-millisecond for local connections
- **Network**: Efficient poll()-based multiplexing

---

## Security Considerations

### Current Implementation

- ✓ Password authentication required
- ✓ Operator privilege separation
- ✓ Input validation (nicknames, usernames, channels)
- ✓ No buffer overflows (std::string usage)
- ✓ Proper socket error handling

### Known Limitations

- ⚠ Passwords sent in plain text (no TLS/SSL)
- ⚠ No rate limiting
- ⚠ No flood protection
- ⚠ No connection limiting
- ⚠ No IP-based restrictions

### Production Recommendations

For production deployment, consider:
- TLS/SSL encryption (IRC over TLS)
- Rate limiting per client
- Flood protection measures
- Connection limits per IP
- Password hashing (bcrypt, Argon2)
- Audit logging
- User authentication backend

---

## Development

### Adding New Commands

1. Declare function in `includes/Commands.hpp`
2. Implement in `sources/Commands.cpp`
3. Add to `processCommand()` in `sources/Server.cpp`
4. Document in `docs/README_COMMANDS.md`

### Adding New Channel Modes

1. Add member variables to `Channel` class
2. Add getters/setters
3. Implement in MODE command handler
4. Document mode behavior

### Debugging

```bash
# Run with GDB
gdb --args ./ircserv 6667 test

# Run with Valgrind
valgrind --leak-check=full ./ircserv 6667 test

# Monitor connections
netstat -an | grep :6667

# View logs (if implemented)
tail -f server.log
```

---

## Contributing

When contributing to this project:

1. Follow existing code style
2. Add documentation for new features
3. Update test scripts as needed
4. Ensure all tests pass before submission
5. Check for memory leaks with Valgrind
6. Document any protocol deviations

---

## License

This project is an educational implementation for the 42 Network curriculum. Refer to your institution's license requirements.

---

## Authors

- **Original Author**: aal-hawa

---

## Acknowledgments

- 42 Network for the curriculum and guidance
- IRC protocol developers (RFC 1459, RFC 2812)
- Open source community for IRC server implementations

---

## Resources

- [RFC 1459 - IRC Protocol](https://tools.ietf.org/html/rfc1459)
- [RFC 2812 - IRC Client Protocol](https://tools.ietf.org/html/rfc2812)
- [IRC Wikipedia](https://en.wikipedia.org/wiki/Internet_Relay_Chat)
- [Modern IRC (IRCv3)](https://ircv3.net/)

---

## Support

For issues, questions, or contributions:
- Check the documentation in `docs/` directory
- Review test scripts for usage examples
- Refer to RFC specifications for protocol details

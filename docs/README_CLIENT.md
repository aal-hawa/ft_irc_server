# Client Class Documentation

## Overview

The `Client` class represents a connected IRC client. It encapsulates all client-related information including authentication status, user details, message buffers, and socket communication. This class handles both incoming data reception and outgoing message transmission.

## Purpose

The Client class serves as an abstraction layer between the IRC server and individual client connections. It manages:
- Client connection state (registered, authenticated, operator status)
- User identity (nickname, username, realname, hostname)
- Message buffers for reliable communication
- Socket communication with the client

## Architecture

The Client class follows a state-machine pattern where clients transition through different states:
1. **Connected** - Socket established but not authenticated
2. **Authenticated** - Password verified but not registered
3. **Registered** - Nickname and username set, fully functional

## Private Member Variables

| Variable | Type | Description |
|----------|------|-------------|
| `_fd` | `int` | File descriptor for the client socket |
| `_nickname` | `std::string` | Client's nickname (up to 9 characters) |
| `_username` | `std::string` | Client's username (up to 9 characters) |
| `_hostname` | `std::string` | Client's hostname/IP address |
| `_realname` | `std::string` | Client's real name |
| `_recvBuffer` | `std::string` | Buffer for incoming data |
| `_sendBuffer` | `std::deque<std::string>` | Queue for outgoing messages |
| `_isRegistered` | `bool` | Whether client has completed registration |
| `_isOperator` | `bool` | Whether client has server operator privileges |
| `_isAuthenticated` | `bool` | Whether client provided correct password |
| `_server` | `Server*` | Pointer to the server instance |

## Public Methods

### Constructors and Destructors

#### `Client(int fd, const std::string& hostname, Server* server)`

**Purpose**: Initializes a new Client object with the given connection details.

**Parameters**:
- `fd` - File descriptor of the connected socket
- `hostname` - Initial hostname (will be replaced with actual client IP)
- `server` - Pointer to the Server instance

**Behavior**:
- Initializes all member variables
- Attempts to retrieve the client's actual IP address using `getpeername()`
- Uses `getnameinfo()` to convert the IP address to a string
- Sets initial state: not registered, not operator, not authenticated
- Uses NI_NUMERICHOST flag to get numeric IP address

#### `~Client()`

**Purpose**: Cleanly closes the client connection.

**Behavior**:
- Closes the socket file descriptor if valid
- Prevents resource leaks
- Does not delete the client from server (handled by Server class)

### Getters

#### `int getFd() const`
Returns the file descriptor of the client's socket.

#### `std::string getNickname() const`
Returns the client's current nickname. Returns empty string if not set yet.

#### `std::string getUsername() const`
Returns the client's username. Returns empty string if not set yet.

#### `std::string getHostname() const`
Returns the client's hostname or IP address. Automatically resolved to IP address upon connection.

#### `std::string getRealname() const`
Returns the client's real name. Can contain spaces and special characters.

#### `bool isRegistered() const`
Returns true if the client has completed registration.

**Registration Criteria**:
- Must be authenticated (correct password)
- Must have set a nickname
- Must have set a username

#### `bool isOperator() const`
Returns true if the client has operator privileges. Operators can use privileged commands (KICK, MODE changes, etc.).

#### `bool isAuthenticated() const`
Returns true if the client provided the correct server password. Required before registration is possible.

#### `std::string getPrefix() const`
Returns the full IRC prefix for this client.

**Format**: `nickname!username@hostname`

**Example**: `alice!user1@192.168.1.100`

**Special Cases**:
- If nickname is empty, uses "*"
- If username is empty, uses "*"

### Setters

#### `void setNickname(const std::string& nick)`
Sets the client's nickname. Validation is handled by Commands module.

#### `void setUsername(const std::string& username)`
Sets the client's username. Should be validated by caller.

#### `void setRealname(const std::string& realname)`
Sets the client's real name. Typically the last parameter of the USER command.

#### `void setRegistered(bool registered)`
Sets the registration status. Set to true after successful NICK and USER commands.

#### `void setOperator(bool isOp)`
Sets or removes operator privileges. Can be set via MODE command in channels.

#### `void setAuthenticated(bool auth)`
Sets the authentication status. Set to true after successful PASS command with correct password.

### Message Handling

#### `void sendToClient(const std::string& message)`
Sends a message to the client. Appends "\r\n" (CRLF) to the message (IRC protocol requirement).

#### `void appendRecvBuffer(const std::string& data)`
Appends received data to the receive buffer. Handles partial messages that may arrive fragmented.

#### `bool hasCompleteMessage() const`
Returns true if the receive buffer contains at least one complete message (ends with "\r\n").

#### `std::string getNextMessage()`
Returns the next complete message from the buffer, or empty string if none. Removes the extracted message from buffer.

## Client Lifecycle

1. **Connection Establishment**: Socket created → Client object created → Hostname resolved
2. **Authentication Phase**: Client connected → PASS command → `_isAuthenticated` set
3. **Registration Phase**: Authenticated → NICK command → USER command → `_isRegistered` = true → Welcome messages sent
4. **Active Phase**: Registered → Send/receive messages → Join/leave channels → Mode changes
5. **Disconnection**: QUIT command or socket error → Removed from all channels → Socket closed → Client deleted

## Usage Example

```cpp
// Creating a new client
int clientFd = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
Client* client = new Client(clientFd, "unknown", server);
server->addClient(client);

// Handling messages
void handleClientData(Client* client, const char* data, size_t len) {
    client->appendRecvBuffer(std::string(data, len));

    while (client->hasCompleteMessage()) {
        std::string message = client->getNextMessage();
        processCommand(client, message);
    }
}

// Sending messages
void sendWelcome(Client* client) {
    client->sendToClient("001 " + client->getNickname() + " :Welcome to IRC");
    client->sendToClient("002 " + client->getNickname() + " :Your host is server");
}
```

## IRC Protocol Compliance

This class follows RFC 1459 and RFC 2812 specifications:
- Prefix format: `nickname!username@hostname` ✓
- Message termination: All messages end with CRLF ✓
- Registration: Requires PASS, NICK, and USER commands ✓
- Nicknames: Limited to 9 characters ✓
- Usernames: Limited to 9 characters ✓

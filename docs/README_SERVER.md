# Server Class Documentation

## Overview

The `Server` class is the core component of the IRC (Internet Relay Chat) server implementation. It handles all server-side operations including socket management, client connections, channel management, and message routing. This class implements a non-blocking, event-driven architecture using the `poll()` system call to handle multiple concurrent connections efficiently.

## Architecture

The Server class follows a typical event-driven server pattern:
- Uses `poll()` for multiplexed I/O operations
- Maintains a map of connected clients indexed by file descriptors
- Manages a collection of active channels
- Implements non-blocking socket operations for scalability

## Private Member Variables

| Variable | Type | Description |
|----------|------|-------------|
| `_serverSocket` | `int` | The main listening socket file descriptor |
| `_port` | `std::string` | The port number on which the server listens |
| `_password` | `std::string` | Server password required for client authentication |
| `_hostname` | `std::string` | Server hostname (default: "localhost") |
| `_creationTime` | `std::string` | Timestamp when the server was created |
| `_pollFds` | `std::vector<struct pollfd>` | Array of file descriptors to monitor with poll() |
| `_clients` | `std::map<int, Client*>` | Map of connected clients indexed by their file descriptors |
| `_channels` | `std::vector<Channel*>` | Collection of active channels |
| `_running` | `bool` | Flag indicating whether the server is running |

## Public Methods

### Constructors and Destructors

#### `Server(const std::string& port, const std::string& password)`

**Purpose**: Initializes and configures the IRC server.

**Parameters**:
- `port` - The port number to listen on (as string)
- `password` - The password required for client authentication

**Behavior**:
- Sets hostname to "localhost"
- Records the creation time
- Creates and configures the server socket
- Sets the socket to non-blocking mode
- Begins listening for incoming connections
- Adds the server socket to the poll monitoring list

#### `~Server()`

**Purpose**: Cleanly shuts down the server and releases all resources.

**Behavior**:
- Stops the server loop
- Closes all client connections and deletes Client objects
- Deletes all Channel objects
- Closes the server socket
- Prevents memory leaks

### Server Lifecycle Management

#### `void run()`

**Purpose**: Starts the main server event loop.

**Behavior**:
- Sets the `_running` flag to true
- Enters the poll loop to handle events
- Blocks execution until `stop()` is called

#### `void stop()`

**Purpose**: Stops the server event loop gracefully.

**Behavior**:
- Sets the `_running` flag to false
- Causes the poll loop to exit on the next iteration

### Getters

#### `std::string getPort() const`

**Returns**: The port number the server is listening on.

#### `std::string getPassword() const`

**Returns**: The server password required for authentication.

#### `std::string getHostname() const`

**Returns**: The server hostname.

### Client Management

#### `void addClient(Client* client)`

**Purpose**: Adds a new client to the server.

**Parameters**:
- `client` - Pointer to the Client object to add

**Behavior**:
- Adds the client to the `_clients` map with file descriptor as key
- Adds the client's socket to the poll monitoring list
- Allows the server to receive data from this client

#### `void removeClient(int fd)`

**Purpose**: Removes a client from the server.

**Parameters**:
- `fd` - The file descriptor of the client to remove

**Behavior**:
- Removes the client's file descriptor from the poll list
- Finds the client in the `_clients` map
- Deletes the Client object (frees memory)
- Erases the client from the map

#### `Client* getClientByFd(int fd) const`

**Purpose**: Retrieves a client by their file descriptor.

**Parameters**:
- `fd` - The file descriptor to search for

**Returns**: Pointer to the Client object, or NULL if not found.

#### `Client* getClientByNickname(const std::string& nickname) const`

**Purpose**: Retrieves a client by their nickname.

**Parameters**:
- `nickname` - The nickname to search for

**Returns**: Pointer to the Client object, or NULL if not found.

**Note**: Iterates through all clients to find a match (O(n) operation).

### Channel Management

#### `void addChannel(Channel* channel)`

**Purpose**: Adds a new channel to the server.

**Parameters**:
- `channel` - Pointer to the Channel object to add

**Behavior**:
- Adds the channel to the `_channels` vector
- Does not check for duplicates

#### `void removeChannel(Channel* channel)`

**Purpose**: Removes a channel from the server.

**Parameters**:
- `channel` - Pointer to the Channel object to remove

**Behavior**:
- Finds the channel in the `_channels` vector
- Removes it from the vector
- Does not delete the Channel object (caller must handle deletion)

#### `Channel* getChannel(const std::string& name) const`

**Purpose**: Retrieves a channel by its name.

**Parameters**:
- `name` - The channel name to search for

**Returns**: Pointer to the Channel object, or NULL if not found.

**Note**: Case-sensitive search (O(n) operation).

#### `std::vector<Channel*> getChannelsByClient(Client* client) const`

**Purpose**: Gets all channels that a client is a member of.

**Parameters**:
- `client` - The client to search for

**Returns**: Vector of Channel pointers where the client is a member.

### Message Broadcasting

#### `void broadcastToChannel(Channel* channel, const std::string& message, Client* exclude)`

**Purpose**: Sends a message to all members of a channel except one client.

**Parameters**:
- `channel` - The channel to broadcast to
- `message` - The message to send
- `exclude` - Client to exclude from the broadcast (NULL to send to all)

**Behavior**:
- Iterates through all channel members
- Sends the message to each member except the excluded client
- Used for announcements, user joins/parts, etc.

### Server Responses

#### `void sendWelcome(Client* client)`

**Purpose**: Sends IRC welcome messages to a newly registered client.

**Parameters**:
- `client` - The client to welcome

**Behavior**:
- Sends RPL_WELCOME (001) with client's full prefix
- Sends RPL_YOURHOST (002) with server hostname and version
- Sends RPL_CREATED (003) with server creation time
- Sends RPL_MYINFO (004) with server information

**IRC Protocol Compliance**:
- Follows RFC 1459 and RFC 2812 specifications for welcome messages

#### `void sendNames(Client* client, Channel* channel)`

**Purpose**: Sends the list of users in a channel to a client.

**Parameters**:
- `client` - The client requesting the names list
- `channel` - The channel whose members to list

**Behavior**:
- Builds a string of all member nicknames
- Prefixes operators with '@' symbol
- Sends RPL_NAMREPLY (353) with the names list
- Sends RPL_ENDOFNAMES (366) to indicate end of list

## Private Methods

### Socket Initialization

#### `void createSocket()`

**Purpose**: Creates and configures the server socket.

**Behavior**:
- Creates a TCP/IP socket (AF_INET, SOCK_STREAM)
- Sets SO_REUSEADDR option to allow quick restart
- Binds the socket to the specified port (INADDR_ANY)
- Throws exceptions on failure

**Socket Configuration**:
- Address family: AF_INET (IPv4)
- Type: SOCK_STREAM (TCP)
- Reuse address: Enabled

#### `void setNonBlocking(int fd)`

**Purpose**: Sets a file descriptor to non-blocking mode.

**Parameters**:
- `fd` - The file descriptor to configure

**Behavior**:
- Gets current file descriptor flags using fcntl()
- Adds O_NONBLOCK flag
- Throws exceptions on failure

**Why Non-Blocking?**:
- Allows the server to handle many clients without blocking
- Works with poll() for efficient I/O multiplexing

#### `void listenForConnections()`

**Purpose**: Starts listening for incoming connections.

**Behavior**:
- Calls listen() with backlog of 10
- Throws exception on failure
- Enables accept() to be called

### Connection Handling

#### `void acceptNewConnection()`

**Purpose**: Accepts a new client connection.

**Behavior**:
- Accepts the next pending connection
- Sets the new client socket to non-blocking mode
- Creates a new Client object
- Adds the client to the server
- Handles non-blocking errors (EAGAIN, EWOULDBLOCK)

#### `void runPollLoop()`

**Purpose**: Main event loop that monitors file descriptors for activity.

**Behavior**:
- Continuously calls poll() to monitor all file descriptors
- Handles EINTR (interrupted system call) gracefully
- On POLLIN event:
  - If server socket: accepts new connection
  - If client socket: processes client data
- Exits when `_running` is false

#### `void handleClientData(int clientFd)`

**Purpose**: Receives and processes data from a client.

**Parameters**:
- `clientFd` - The client's file descriptor

**Behavior**:
- Receives up to 1023 bytes of data
- Appends data to client's receive buffer
- Processes all complete messages in the buffer
- Handles client disconnection (bytesRead <= 0)

#### `void handleClientDisconnect(int clientFd)`

**Purpose**: Handles a client disconnection.

**Parameters**:
- `clientFd` - The disconnecting client's file descriptor

**Behavior**:
- Executes QUIT command for the client
- Removes client from all channels
- Cleans up channel if empty
- Removes client from server

### Command Processing

#### `void processCommand(Client* client, const std::string& messageStr)`

**Purpose**: Parses and dispatches IRC commands.

**Parameters**:
- `client` - The client that sent the command
- `messageStr` - The raw message string

**Behavior**:
- Creates a Message object to parse the raw string
- Converts command to uppercase for case-insensitive comparison
- Dispatches to appropriate command handler:
  - PASS - Password authentication
  - NICK - Change/set nickname
  - USER - Set user information
  - JOIN - Join a channel
  - PART - Leave a channel
  - PRIVMSG - Send private or channel message
  - QUIT - Disconnect from server
  - KICK - Remove user from channel
  - INVITE - Invite user to channel
  - TOPIC - View or set channel topic
  - MODE - Change channel or user modes
- Sends error (421) for unknown commands

## Usage Example

```cpp
#include "Server.hpp"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
        return 1;
    }

    try {
        Server server(argv[1], argv[2]);
        std::cout << "IRC Server started on port " << argv[1] << std::endl;
        server.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```

## Error Handling

The Server class uses C++ exceptions for error handling:
- `std::runtime_error` for socket creation and configuration failures
- Validates client operations before execution
- Gracefully handles client disconnections
- Logs appropriate IRC error messages to clients

## Thread Safety

**Note**: This implementation is not thread-safe. The Server class is designed to run in a single thread with an event-driven architecture. For multi-threaded use, additional synchronization would be required.

## Performance Considerations

- Uses poll() instead of select() for better scalability
- Non-blocking I/O prevents single slow clients from blocking others
- Linear searches (getClientByNickname, getChannel) could be optimized with hash maps for large deployments
- Efficient memory management with proper cleanup in destructor

## Dependencies

- `poll.h` - poll() system call
- `sys/socket.h` - Socket operations
- `netinet/in.h` - Internet address structures
- `arpa/inet.h` - IP address conversion
- `unistd.h` - close()
- `fcntl.h` - File descriptor manipulation
- `Client.hpp` - Client class
- `Channel.hpp` - Channel class
- `Commands.hpp` - IRC command handlers
- `Utils.hpp` - Utility functions

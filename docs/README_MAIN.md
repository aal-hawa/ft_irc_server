# Main Entry Point Documentation

## Overview

`main.cpp` is the entry point for the IRC server application. It initializes the server, sets up signal handling for graceful shutdown, and manages the server lifecycle. This is the first code executed when the program starts.

## Purpose

The main function serves as the application bootstrap:
- Validates command-line arguments
- Initializes the IRC Server instance
- Sets up signal handlers for graceful shutdown
- Runs the server event loop
- Handles errors and clean shutdown

## Global Variables

### `Server* g_server`

**Type**: Pointer to Server object

**Purpose**: Global pointer to the server instance for use in signal handler.

**Why Global**: Signal handlers require a globally accessible pointer since they receive limited arguments.

**Lifetime**: Set after server construction, valid until program exit.

## Functions

---

### `signalHandler(int signum)`

**Purpose**: Handles operating system signals for graceful shutdown.

**Parameters**:
- `signum` - The signal number received

**Behavior**:
1. Stops the server by calling `g_server->stop()`
2. Signals the poll loop to exit
3. Allows the server to clean up resources properly

**Handled Signals**:
- **SIGINT** (2) - Interrupt signal (Ctrl+C)
- **SIGTERM** (15) - Termination signal (kill command)

**Implementation**:
```cpp
void signalHandler(int signum) {
    (void)signum;  // Suppress unused parameter warning
    if (g_server) {
        std::cout << "\nShutting down server..." << std::endl;
        g_server->stop();
    }
}
```

**Usage Flow**:
```
User presses Ctrl+C
    ↓
SIGINT signal sent to process
    ↓
signalHandler() called
    ↓
g_server->stop() called
    ↓
Server poll loop exits
    ↓
main() continues
    ↓
Server destructor called (cleanup)
    ↓
Program exits
```

---

### `int main(int argc, char* argv[])`

**Purpose**: Entry point of the IRC server application.

**Parameters**:
- `argc` - Argument count (number of command-line arguments)
- `argv` - Argument vector (array of argument strings)

**Returns**: Exit code (0 for success, 1 for error)

---

## Command-Line Interface

### Syntax

```bash
./ircserv <port> <password>
```

### Parameters

| Parameter | Type | Required | Description |
|-----------|------|----------|-------------|
| `port` | string | Yes | Port number for server to listen on |
| `password` | string | Yes | Server password for client authentication |

### Examples

```bash
# Start server on port 6667 with password "testpass"
./ircserv 6667 testpass

# Start server on port 8080 with password "secret"
./ircserv 8080 secret

# Start server on port 9000
./ircserv 9000 mypassword
```

---

## Execution Flow

### 1. Argument Validation
```cpp
if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <port> <password>" << std::endl;
    return 1;
}
```

**Behavior**:
- Checks if exactly 3 arguments provided (program name + port + password)
- Prints usage message if incorrect
- Exits with error code 1

**Error Output**:
```
Usage: ./ircserv <port> <password>
```

---

### 2. Argument Processing
```cpp
std::string port = argv[1];
std::string password = argv[2];
```

**Behavior**:
- Extracts port from second argument
- Extracts password from third argument
- Stores as strings for later use

**Note**: Port string is converted to integer by Server class.

---

### 3. Signal Handler Setup
```cpp
signal(SIGINT, signalHandler);
signal(SIGTERM, signalHandler);
```

**Behavior**:
- Registers signalHandler() for SIGINT (Ctrl+C)
- Registers signalHandler() for SIGTERM (kill command)
- Enables graceful shutdown on these signals

**Signal Handling Strategy**:
- Minimal work in signal handler (stop server only)
- Main cleanup in normal execution flow
- Avoids async-signal-unsafe operations

---

### 4. Server Initialization and Execution
```cpp
try {
    Server server(port, password);
    g_server = &server;

    std::cout << "IRC Server started on port " << port << std::endl;
    server.run();

} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
}
```

**Behavior**:
1. Creates Server object with port and password
2. Sets global pointer for signal handler
3. Prints startup message
4. Runs server event loop (blocking call)
5. Catches and reports exceptions

**Server Lifecycle**:
```
Server() constructor called
    ↓
Socket created, configured, bound
    ↓
Server ready to accept connections
    ↓
server.run() called (blocks)
    ↓
    [Server handles connections, commands]
    ↓
Signal received or stop() called
    ↓
server.run() returns
    ↓
Server destructor called (cleanup)
    ↓
main() returns 0 (success)
```

---

### 5. Error Handling
```cpp
catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
}
```

**Behavior**:
- Catches all standard exceptions
- Prints error message to stderr
- Exits with error code 1

**Common Exceptions**:
- `std::runtime_error` - Socket creation/binding failures
- `std::exception` - Other unexpected errors

---

## Complete Execution Example

### Normal Startup and Shutdown

```bash
$ ./ircserv 6667 testpass
IRC Server started on port 6667
[Server running, handling clients...]
^C
Shutting down server...
$
```

### Error: Missing Arguments

```bash
$ ./ircserv
Usage: ./ircserv <port> <password>
$ echo $?
1
```

### Error: Socket Creation Failure

```bash
$ ./ircserv 99999 testpass
Error: Failed to bind socket
$ echo $?
1
```

---

## Graceful Shutdown Process

### Signal Received
```
User sends SIGINT (Ctrl+C) or SIGTERM
    ↓
signalHandler() executes
    ↓
Prints "Shutting down server..."
    ↓
Calls g_server->stop()
    ↓
Sets _running = false
```

### Server Cleanup
```
poll() loop exits
    ↓
server.run() returns
    ↓
Server destructor runs:
    - Stops server
    - Closes all client connections
    - Deletes all clients
    - Deletes all channels
    - Closes server socket
    ↓
main() returns 0
```

---

## Error Scenarios

### 1. Invalid Port Number
```bash
$ ./ircserv abc testpass
# Server may fail to bind or use port 0
# Error handling depends on Server implementation
```

### 2. Port Already in Use
```bash
$ ./ircserv 6667 testpass
Error: Failed to bind socket
$ echo $?
1
```

### 3. Insufficient Permissions (Port < 1024)
```bash
$ ./ircserv 66 testpass
Error: Failed to bind socket
$ echo $?
1
```

**Note**: Ports below 1024 require root privileges on Unix systems.

---

## Integration with Other Components

### Server Class
- Created in main()
- Used via `run()` method
- Cleaned up via destructor

### Signal Handler
- Accesses global server pointer
- Calls `stop()` method
- Minimal signal-safe operations

### Client Connections
- Managed internally by Server class
- Not directly accessed from main()

---

## Best Practices

### 1. Always Validate Arguments
```cpp
if (argc != 3) {
    // Print usage and exit
}
```

### 2. Use Exceptions for Errors
```cpp
try {
    Server server(port, password);
    server.run();
} catch (const std::exception& e) {
    // Handle error
}
```

### 3. Minimal Signal Handler
```cpp
void signalHandler(int signum) {
    (void)signum;
    if (g_server) {
        g_server->stop();  // Only set flag
    }
}
```

### 4. Clear User Feedback
```cpp
std::cout << "IRC Server started on port " << port << std::endl;
std::cerr << "Error: " << e.what() << std::endl;
```

---

## Security Considerations

1. **Password Handling**:
   - Password stored as string in memory
   - Cleared from memory when server exits
   - No password hashing (may be security issue for production)

2. **Port Validation**:
   - Port validation done by Server class
   - Consider validating port range in main()

3. **Signal Safety**:
   - Signal handler only sets flag
   - No async-signal-unsafe operations
   - Cleanup in normal code path

---

## Deployment

### Running as Daemon

For production deployment, you might want to run the server as a background process:

```bash
# Using nohup
nohup ./ircserv 6667 testpass > server.log 2>&1 &

# Using screen
screen -S ircserver
./ircserv 6667 testpass
# Ctrl+A, D to detach

# Using systemd (production)
# Create systemd service file
sudo systemctl start ircserver
sudo systemctl enable ircserver  # Start on boot
```

### Monitoring

```bash
# Check if server is running
ps aux | grep ircserv

# Monitor logs
tail -f server.log

# Check port is listening
netstat -tlnp | grep ircserv
```

---

## Dependencies

- `Server.hpp` - Server class definition
- `iostream` - Standard I/O streams (cout, cerr)
- `cstdlib` - General utilities
- `signal.h` - Signal handling functions

---

## Testing

### Manual Testing
```bash
# Start server in one terminal
./ircserv 6667 testpass

# Connect with netcat in another terminal
nc localhost 6667
PASS testpass
NICK testuser
USER testuser 0 * :Test User
```

### Automated Testing
```bash
# Run test scripts
./scripts/test_basic_commands.sh
./scripts/test_channel_operations.sh
./scripts/final_test.sh
```

---

## Future Enhancements

Possible improvements to main.cpp:

1. **Configuration File**: Load port/password from config file instead of command line
2. **Logging**: Implement proper logging system (file, syslog)
3. **Daemon Mode**: Option to run as background daemon
4. **Command-Line Options**: More options (--port, --password, --verbose, etc.)
5. **PID File**: Write PID file for process management
6. **Reload Signal**: Support SIGHUP for config reload
7. **Status Command**: Query server status via signal or pipe

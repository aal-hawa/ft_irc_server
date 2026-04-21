*This project has been created as part of the 42 curriculum by aal-hawa (Hassan, Tamim).*

# ft_irc

## Description

**ft_irc** is an IRC (Internet Relay Chat) server implemented in C++ 98. The project was developed as part of the 42 Abu Dhabi curriculum to deepen understanding of network programming, the TCP/IP protocol stack, and the IRC communication protocol defined in RFC 2812.

The server listens on a specified port for incoming IRC client connections and handles multiple clients simultaneously using a single `poll()` call with non-blocking file descriptors. It supports the full set of mandatory IRC features required by the project specification: user authentication (PASS), nickname and username registration (NICK, USER), channel management (JOIN, PART, INVITE, KICK, TOPIC, MODE), and private messaging (PRIVMSG, NOTICE). Channel operator privileges are enforced through the MODE command with support for the `i` (invite-only), `t` (topic restricted), `k` (channel key), `o` (operator), and `l` (user limit) mode flags.

The server is designed to be compatible with standard IRC clients such as **irssi**, which serves as the reference client for evaluation. All communication follows the IRC message format with proper numeric replies as defined in RFC 2812. The implementation handles partial message reassembly, ensuring that commands split across multiple TCP packets are correctly reconstructed before processing.

### Key Features

- **Non-blocking I/O**: All socket operations use non-blocking file descriptors managed by a single `poll()` loop, ensuring the server never hangs while handling multiple clients.
- **Multi-client support**: The server can handle many simultaneous connections without forking, using an event-driven architecture.
- **Full mandatory command set**: PASS, NICK, USER, QUIT, JOIN, PART, KICK, INVITE, TOPIC, MODE, PRIVMSG, NOTICE.
- **Channel modes**: `+i` (invite-only), `+t` (topic restricted to operators), `+k` (channel key/password), `+o` (channel operator), `+l` (user limit).
- **Partial message handling**: Correctly reassembles IRC messages that arrive in fragmented TCP packets, including support for both `\r\n` and `\n` line endings.
- **irssi compatible**: Tested and verified to work with the irssi IRC client as the reference implementation.

## Instructions

### Prerequisites

- A C++ compiler supporting C++ 98 (g++ or clang++)
- Make

### Compilation

Clone the repository and build the project:

```bash
git clone https://github.com/aal-hawa/ft_irc_server.git
cd ft_irc_server
make
```

The Makefile supports the following rules:

| Rule | Description |
|------|-------------|
| `make` or `make all` | Compiles the project and produces the `ircserv` executable |
| `make clean` | Removes object files |
| `make fclean` | Removes object files and the executable |
| `make re` | Performs `fclean` followed by `all` |

### Running the Server

Start the server by providing a port number and a connection password:

```bash
./ircserv <port> <password>
```

- **port**: The TCP port number on which the server will listen for incoming IRC connections (e.g., `6667`).
- **password**: The password that IRC clients must provide using the `PASS` command before registering.

Example:

```bash
./ircserv 6667 secretpass
```

### Connecting with an IRC Client

Using **irssi** as the reference client:

```bash
irssi -c 127.0.0.1 -p 6667 -w secretpass -n yournick
```

Or using **nc** (netcat) for raw testing:

```bash
nc -C 127.0.0.1 6667
PASS secretpass
NICK yournick
USER yournick 0 * :Your Name
JOIN #testchannel
PRIVMSG #testchannel :Hello everyone!
```

### Testing Partial Message Handling

As specified in the subject, you can test that the server correctly handles partial data by sending a command in fragments using `Ctrl+D` in nc:

```bash
nc -C 127.0.0.1 6667
com^Dman^Dd
```

Where `^D` is `Ctrl+D`, sending "com", then "man", then "d\n" — the server should reassemble these into the full command "command".

## Resources

### IRC Protocol Documentation

- [RFC 2812 - Internet Relay Chat: Client Protocol](https://www.rfc-editor.org/rfc/rfc2812): The primary reference for IRC client-server communication, message format, numeric replies, and command specifications.
- [RFC 2813 - Internet Relay Chat: Server Protocol](https://www.rfc-editor.org/rfc/rfc2813): Reference for server-to-server communication (not implemented in this project, but useful for understanding the full IRC architecture).
- [RFC 1459 - Internet Relay Chat Protocol](https://www.rfc-editor.org/rfc/rfc1459): The original IRC protocol specification, superseded by RFC 2812 but still valuable for historical context and foundational concepts.

### Network Programming

- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/): A comprehensive and accessible guide to socket programming in C/C++, covering TCP/IP fundamentals, `poll()`/`select()`, and non-blocking I/O.
- [Linux `poll()` man page](https://man7.org/linux/man-pages/man2/poll.2.html): Official documentation for the `poll()` system call used for I/O multiplexing.

### IRC Client

- [irssi - The client of the future](https://irssi.org/): The reference IRC client used for testing this server. Open-source, terminal-based, and widely available on Linux and macOS.

### AI Usage

AI tools were used during this project primarily for the following tasks:

- **Understanding RFC 2812**: Assisting in interpreting specific sections of the IRC protocol specification, particularly the numeric reply formats and the expected behavior of each command.
- **Debugging numeric reply formatting**: Helping identify cases where server responses did not conform to the `:servername numeric nick ...` format required by RFC 2812.
- **Verifying edge cases**: Discussing corner cases such as partial message handling, nickname case-insensitivity rules per IRC convention, and the correct behavior of the INVITE command on invite-only versus non-invite-only channels.
- **Code review**: Reviewing command implementations for correctness against the specification, such as ensuring NOTICE never sends error replies and MODE broadcasts include all parameter values.

All AI-generated suggestions were thoroughly reviewed, tested, and validated before being incorporated into the final codebase.

# Message Class Documentation

## Overview

The `Message` class is responsible for parsing raw IRC protocol messages into structured components. IRC messages follow a specific format defined in RFC 1459, and this class handles the extraction of the prefix, command, parameters, and trailing parameter from raw message strings.

## Purpose

The Message class serves as the message parsing layer of the IRC server:
- Parses raw IRC messages from clients
- Extracts message components according to IRC protocol
- Validates message completeness
- Provides easy access to message parts

## IRC Message Format

According to RFC 1459, IRC messages have the following format:

```
[":" prefix " "] command [ params ] crlf
params     =  *14( space middle ) [ space ":" trailing ]
middle     =  nospcrlfcl *( ":" / nospcrlfcl )
trailing   =  *( ":" / " " / nospcrlfcl )
crlf       =  CR LF
```

### Examples:
```
:alice!user@host PRIVMSG #general :Hello everyone!
NICK newnick
JOIN #channel
TOPIC #general :New topic here
:server MODE #channel +i
```

## Private Member Variables

| Variable | Type | Description |
|----------|------|-------------|
| `_raw` | `std::string` | The raw message string (modified during parsing) |
| `_prefix` | `std::string` | Message prefix (sender identifier) |
| `_command` | `std::string` | IRC command or numeric reply |
| `_params` | `std::vector<std::string>` | Command parameters (excluding trailing) |
| `_trailing` | `std::string` | Trailing parameter (after colon) |
| `_complete` | `bool` | Whether the message is complete (has CRLF) |

## Public Methods

---

### Constructors

#### `Message(const std::string& raw)`

**Purpose**: Constructs a Message object and parses the raw message.

**Parameters**:
- `raw` - The raw message string received from the client

**Behavior**:
1. Stores the raw message
2. Checks for CRLF (`\r\n`) at the end
3. If CRLF found:
   - Marks message as complete
   - Removes CRLF from raw string
   - Calls `parse()` to extract components
4. If CRLF not found:
   - Marks message as incomplete
   - Does not parse

**Example**:
```cpp
Message msg1("NICK alice\r\n");              // Complete message
Message msg2("NICK alice");                   // Incomplete message
Message msg3(":alice!user@host PRIVMSG #test :Hello\r\n"); // Complete
```

---

### Message Parsing

#### `void parse()`

**Purpose**: Parses the raw message into its components.

**Behavior**:
1. Calls `extractPrefix()` to get the sender prefix
2. Calls `extractCommandAndParams()` to get command and parameters
3. Stores results in member variables

**Note**: Should only be called if message is complete.

---

### Validation

#### `bool isComplete() const`

**Purpose**: Checks if the message is complete.

**Returns**: `true` if message has CRLF terminator, `false` otherwise.

**Usage**: Called before parsing to ensure message is complete.

**Example**:
```cpp
Message msg("NICK alice\r\n");
if (msg.isComplete()) {
    msg.parse();
    // Process message
}
```

---

### Getters

#### `std::string getPrefix() const`

**Returns**: The message prefix (sender identifier).

**Format**: `nickname!username@hostname` or `servername`

**Example**:
```
Input:  :alice!user@host PRIVMSG #test :Hello
Output: "alice!user@host"
```

**Note**: Returns empty string if no prefix.

---

#### `std::string getCommand() const`

**Returns**: The IRC command or numeric reply.

**Example**:
```
Input:  NICK alice
Output: "NICK"

Input:  001 alice :Welcome
Output: "001"
```

**Note**: Always lowercase as stored (case conversion done by caller).

---

#### `std::vector<std::string> getParams() const`

**Returns**: Vector of message parameters (excluding trailing).

**Example**:
```
Input:  MODE #channel +i -k
Output: ["MODE", "#channel", "+i", "-k"]
```

**Note**: Trailing parameter is accessed via `getTrailing()`.

---

#### `std::string getTrailing() const`

**Returns**: The trailing parameter (text after colon).

**Example**:
```
Input:  PRIVMSG #channel :This is a message
Output: "This is a message"
```

**Note**: Returns empty string if no trailing parameter.

---

## Private Methods

---

### `void extractPrefix()`

**Purpose**: Extracts the prefix from the raw message.

**Behavior**:
1. Checks if message starts with `:`
2. If yes, finds the first space
3. Extracts text between `:` and space as prefix
4. Removes prefix from raw string
5. If no space found, raw string becomes empty

**Algorithm**:
```
If raw[0] == ':':
    Find first space position
    If space found:
        prefix = raw[1:space]
        raw = raw[space+1:]
    Else:
        raw = ""
```

**Examples**:
```
Input:  ":alice!user@host NICK newnick"
Output: prefix = "alice!user@host", raw = "NICK newnick"

Input:  "NICK newnick"
Output: prefix = "", raw = "NICK newnick"
```

---

### `void extractCommandAndParams()`

**Purpose**: Extracts the command and parameters from the raw message.

**Behavior**:
1. Extract command (text until first space)
2. Extract parameters (space-separated values)
3. Handle trailing parameter (starts with `:`)
4. Store trailing in `_trailing`
5. Store other params in `_params`

**Algorithm**:
```
If raw is not empty:
    Extract command (first word)
    While raw is not empty:
        If starts with ':':
            trailing = raw[1:]
            Add trailing to params
            Break
        Find next space
        If space found:
            param = raw[:space]
            Add param to params
            raw = raw[space+1:]
        Else:
            If raw not empty:
                Add raw to params
            Break
```

**Examples**:
```
Input: "NICK alice"
Output: command = "NICK", params = ["alice"], trailing = ""

Input: "PRIVMSG #channel :Hello world!"
Output: command = "PRIVMSG", params = ["#channel"], trailing = "Hello world!"

Input: "MODE #channel +i -k key"
Output: command = "MODE", params = ["#channel", "+i", "-k", "key"], trailing = ""

Input: "TOPIC #channel"
Output: command = "TOPIC", params = ["#channel"], trailing = ""
```

---

## Complete Parsing Examples

### Example 1: Simple Command
```
Raw Input: "NICK alice\r\n"

Parsing Steps:
1. Remove CRLF → "NICK alice"
2. Extract prefix → None (doesn't start with :)
3. Extract command → "NICK"
4. Extract params → ["alice"]
5. Extract trailing → ""

Result:
- prefix: ""
- command: "NICK"
- params: ["alice"]
- trailing: ""
```

### Example 2: Command with Prefix
```
Raw Input: ":alice!user@host PRIVMSG #test :Hello world!\r\n"

Parsing Steps:
1. Remove CRLF → ":alice!user@host PRIVMSG #test :Hello world!"
2. Extract prefix → "alice!user@host"
3. Extract command → "PRIVMSG"
4. Extract params → ["#test"]
5. Extract trailing → "Hello world!"

Result:
- prefix: "alice!user@host"
- command: "PRIVMSG"
- params: ["#test"]
- trailing: "Hello world!"
```

### Example 3: Multiple Parameters
```
Raw Input: "MODE #channel +i -l 10\r\n"

Parsing Steps:
1. Remove CRLF → "MODE #channel +i -l 10"
2. Extract prefix → None
3. Extract command → "MODE"
4. Extract params → ["#channel", "+i", "-l", "10"]
5. Extract trailing → ""

Result:
- prefix: ""
- command: "MODE"
- params: ["#channel", "+i", "-l", "10"]
- trailing: ""
```

### Example 4: Server Message
```
Raw Input: ":server001 001 alice :Welcome to IRC\r\n"

Parsing Steps:
1. Remove CRLF → ":server001 001 alice :Welcome to IRC"
2. Extract prefix → "server001"
3. Extract command → "001"
4. Extract params → ["alice"]
5. Extract trailing → "Welcome to IRC"

Result:
- prefix: "server001"
- command: "001"
- params: ["alice"]
- trailing: "Welcome to IRC"
```

---

## Usage Examples

### Parsing a Client Message
```cpp
// Client sends: "NICK alice\r\n"
std::string rawData = client->getNextMessage();
Message message(rawData);

if (message.isComplete()) {
    message.parse();

    std::string cmd = Utils::toUpper(message.getCommand());
    std::vector<std::string> params = message.getParams();

    if (cmd == "NICK" && !params.empty()) {
        std::string nickname = params[0];
        // Process nickname change
    }
}
```

### Handling Messages with Trailing
```cpp
// Client sends: "PRIVMSG #channel :Hello everyone!\r\n"
Message message("PRIVMSG #channel :Hello everyone!\r\n");

std::string cmd = message.getCommand();  // "PRIVMSG"
std::vector<std::string> params = message.getParams();  // ["#channel"]
std::string trailing = message.getTrailing();  // "Hello everyone!"

if (cmd == "PRIVMSG") {
    std::string target = params[0];
    std::string text = trailing;
    // Send message to target
}
```

### Processing Messages with Prefix
```cpp
// Client sends: ":alice!user@host PRIVMSG bob :Hi there!\r\n"
Message message(":alice!user@host PRIVMSG bob :Hi there!\r\n");

std::string prefix = message.getPrefix();  // "alice!user@host"
std::string cmd = message.getCommand();    // "PRIVMSG"

// Extract nickname from prefix
size_t exclPos = prefix.find('!');
std::string nickname = prefix.substr(0, exclPos);  // "alice"
```

---

## Error Handling

### Incomplete Messages
```cpp
Message incompleteMsg("NICK ali");  // No CRLF

if (!incompleteMsg.isComplete()) {
    // Message incomplete, wait for more data
    // Do not attempt to parse
}
```

### Empty Messages
```cpp
Message emptyMsg("\r\n");  // Just CRLF

if (emptyMsg.isComplete()) {
    emptyMsg.parse();
    // All fields will be empty
    // Command should be validated by caller
}
```

---

## IRC Protocol Compliance

This class follows RFC 1459 and RFC 2812 specifications:

✓ Prefix format: `nickname!username@hostname` or servername
✓ Command extraction (first word after prefix)
✓ Parameter parsing (space-separated)
✓ Trailing parameter handling (after colon)
✓ CRLF message termination
✓ Support for server messages with numeric replies

---

## Design Patterns

### Parser Pattern
- Single responsibility: Parse IRC messages
- Immutable after construction (except internal _raw during parsing)
- Clear separation of parsing and validation

### Benefits
1. **Encapsulation**: Parsing logic hidden from user
2. **Validation**: Completeness check before parsing
3. **Convenience**: Easy access to message components
4. **Reusability**: Can be used for both client and server messages

---

## Performance Considerations

1. **String Modifications**:
   - `_raw` is modified during parsing (substrings removed)
   - Each operation creates new string objects
   - Could be optimized with iterators for large messages

2. **Memory Usage**:
   - `params` vector allocates memory for each parameter
   - Suitable for IRC (messages typically < 512 bytes)

3. **Parsing Complexity**:
   - O(n) where n is message length
   - Single pass through message string
   - Efficient for typical IRC usage

---

## Dependencies

- `string` - String manipulation
- `vector` - Parameter storage
- `algorithm` - std::find (implicitly used)

---

## Best Practices

1. **Check Completeness**: Always check `isComplete()` before parsing
2. **Validate Commands**: Convert command to uppercase for comparison
3. **Handle Empty Fields**: Check if prefix/params are empty before use
4. **Trailing Parameter**: Remember that trailing may contain spaces
5. **Error Cases**: Handle messages with missing/invalid components

---

## Limitations

1. **No Validation**: Class doesn't validate command names or parameter formats
2. **No Error Codes**: No mechanism to report parsing errors
3. **Single Message**: One object per message (no streaming)
4. **Case Sensitivity**: Command case preserved (conversion done by caller)

---

## Future Enhancements

Possible additions to the Message class:
1. Validation methods (isValid(), hasRequiredParams())
2. Error reporting (error codes, error messages)
3. Helper methods (getTarget(), getText())
4. Support for IRCv3 message tags
5. Builder pattern for constructing messages

# Utils Class Documentation

## Overview

The `Utils` class provides static utility functions for various common operations throughout the IRC server. It encapsulates validation functions, string manipulation utilities, and helper functions used across multiple modules.

## Purpose

The Utils class serves as a centralized utility library for the IRC server:
- Validates IRC protocol elements (passwords, nicknames, usernames, channel names)
- Provides string manipulation functions (split, uppercase, trim)
- Offers type conversion utilities
- Supplies time-related functions

## Architecture

The Utils class follows a static utility class pattern:
- All methods are static (no instantiation needed)
- No member variables (stateless)
- Self-contained helper functions
- Called as `Utils::functionName()`

## Public Static Methods

---

## Validation Methods

### `bool isValidPassword(const std::string& password)`

**Purpose**: Validates a server password.

**Parameters**:
- `password` - The password string to validate

**Returns**: `true` if valid, `false` otherwise.

**Validation Rules**:
- Password must not be empty

**Example**:
```cpp
if (Utils::isValidPassword(mypass)) {
    // Password is valid
}
```

**Implementation**:
```cpp
return !password.empty();
```

---

### `bool isValidNickname(const std::string& nickname)`

**Purpose**: Validates an IRC nickname according to RFC 1459 specifications.

**Parameters**:
- `nickname` - The nickname to validate

**Returns**: `true` if valid, `false` otherwise.

**Validation Rules**:
- Must not be empty
- Maximum length: 9 characters
- First character must be one of: letter, `[`, `]`, `\`, `^`, `_`, `` ` ``
- Subsequent characters must be: letter, digit, `-`, `[`, `]`, `\`, `^`, `_`, `` ` ``

**RFC 1459 Specification**:
```
nickname   =  ( letter / special ) *8( letter / digit / special / "-" )
special    =  "[" / "]" / "\" / "`" / "_" / "^"
```

**Examples**:
- Valid: `alice`, `User123`, `test_user`, `bob^2`
- Invalid: `123abc` (starts with digit), `verylongnickname` (>9 chars), `user@name` (invalid character)

**Implementation**:
```cpp
if (nickname.empty() || nickname.length() > 9) {
    return false;
}

// First character must be letter or special chars: [ \ ] ^ _ `
if (!isalpha(nickname[0]) &&
    nickname[0] != '[' && nickname[0] != ']' &&
    nickname[0] != '\\' && nickname[0] != '^' &&
    nickname[0] != '_' && nickname[0] != '`') {
    return false;
}

// Rest can be letter, digit, or special chars: - [ \ ] ^ _ `
for (size_t i = 1; i < nickname.length(); ++i) {
    if (!isalnum(nickname[i]) &&
        nickname[i] != '-' && nickname[i] != '[' &&
        nickname[i] != ']' && nickname[i] != '\\' &&
        nickname[i] != '^' && nickname[i] != '_' &&
        nickname[i] != '`') {
        return false;
    }
}

return true;
```

---

### `bool isValidUsername(const std::string& username)`

**Purpose**: Validates a username according to IRC protocol.

**Parameters**:
- `username` - The username to validate

**Returns**: `true` if valid, `false` otherwise.

**Validation Rules**:
- Must not be empty
- Maximum length: 9 characters
- Can contain only: letters, digits, hyphens (`-`)

**Examples**:
- Valid: `alice`, `user123`, `test-user`, `bob-1`
- Invalid: `user@name` (invalid character), `1234567890` (>9 chars), `user name` (contains space)

**Implementation**:
```cpp
if (username.empty() || username.length() > 9) {
    return false;
}

// Username can contain letters, digits, and hyphens
for (size_t i = 0; i < username.length(); ++i) {
    if (!isalnum(username[i]) && username[i] != '-') {
        return false;
    }
}

return true;
```

---

### `bool isValidChannelName(const std::string& channel)`

**Purpose**: Validates a channel name according to IRC protocol.

**Parameters**:
- `channel` - The channel name to validate

**Returns**: `true` if valid, `false` otherwise.

**Validation Rules**:
- Must not be empty
- Maximum length: 200 characters
- Must start with `#` or `&`
- Cannot contain: space, comma, control characters (ASCII 1-31, 127, or BEL (7))

**RFC 1459 Specification**:
```
channel    =  ( "#" / "&" ) chanstring
chanstring =  %x01-07 / %x08-09 / %x0B-0C / %x0E-1F / %x21-2B
             / %x2D-39 / %x3B-FF
```

**Examples**:
- Valid: `#general`, `#help`, `&private`, `#test-channel`
- Invalid: `general` (missing prefix), `#chan nel` (contains space), `#a,b` (contains comma)

**Implementation**:
```cpp
if (channel.empty() || channel.length() > 200) {
    return false;
}

// Channel must start with # or &
if (channel[0] != '#' && channel[0] != '&') {
    return false;
}

// Rest can be any printable character except space, comma, or control chars
for (size_t i = 1; i < channel.length(); ++i) {
    if (channel[i] == ' ' || channel[i] == ',' || channel[i] == 7) {
        return false;
    }
    if (channel[i] < 1 || channel[i] > 31 || channel[i] == 127) {
        return false;
    }
}

return true;
```

---

## String Manipulation Methods

### `std::vector<std::string> split(const std::string& str, char delimiter)`

**Purpose**: Splits a string into substrings based on a delimiter character.

**Parameters**:
- `str` - The string to split
- `delimiter` - The delimiter character

**Returns**: Vector of substrings (non-empty tokens only).

**Behavior**:
- Splits the string at each occurrence of the delimiter
- Omits empty tokens from the result
- Uses stringstream for efficient parsing

**Example**:
```cpp
std::vector<std::string> channels = Utils::split("#chan1,#chan2,#chan3", ',');
// Result: ["#chan1", "#chan2", "#chan3"]
```

**Implementation**:
```cpp
std::vector<std::string> tokens;
std::stringstream ss(str);
std::string token;

while (std::getline(ss, token, delimiter)) {
    if (!token.empty()) {
        tokens.push_back(token);
    }
}

return tokens;
```

**Use Cases**:
- Parsing comma-separated channel lists
- Parsing comma-separated parameter lists
- Breaking up command arguments

---

### `std::string toUpper(const std::string& str)`

**Purpose**: Converts a string to uppercase.

**Parameters**:
- `str` - The string to convert

**Returns**: Uppercase version of the string.

**Behavior**:
- Uses `std::transform` with `::toupper`
- Returns a new string (original unchanged)
- Case conversion follows locale settings

**Example**:
```cpp
std::string cmd = Utils::toUpper("join");
// Result: "JOIN"
```

**Implementation**:
```cpp
std::string result = str;
std::transform(result.begin(), result.end(), result.begin(), ::toupper);
return result;
```

**Use Cases**:
- Case-insensitive command comparison
- Normalizing channel names (IRC is case-insensitive for channel names)

---

### `void trim(std::string& str)`

**Purpose**: Removes leading and trailing whitespace from a string (in-place).

**Parameters**:
- `str` - The string to trim (modified in-place)

**Behavior**:
- Removes leading whitespace (spaces, tabs, CR, LF)
- Removes trailing whitespace
- Modifies the string directly (no return value)

**Whitespace Characters**:
- Space (` `)
- Tab (`\t`)
- Carriage return (`\r`)
- Line feed (`\n`)

**Example**:
```cpp
std::string text = "  Hello World  \n";
Utils::trim(text);
// text is now "Hello World"
```

**Implementation**:
```cpp
// Trim leading whitespace
size_t start = str.find_first_not_of(" \t\r\n");
if (start != std::string::npos) {
    str = str.substr(start);
}

// Trim trailing whitespace
size_t end = str.find_last_not_of(" \t\r\n");
if (end != std::string::npos) {
    str = str.substr(0, end + 1);
}
```

**Use Cases**:
- Cleaning user input
- Normalizing command parameters
- Preparing strings for processing

---

## Type Conversion Methods

### `int atoi(const std::string& str)`

**Purpose**: Converts a string to an integer.

**Parameters**:
- `str` - The string to convert

**Returns**: Integer value of the string.

**Behavior**:
- Uses stringstream for conversion
- Returns 0 if conversion fails
- Does not throw exceptions

**Example**:
```cpp
int port = Utils::atoi("6667");
// Result: 6667

int invalid = Utils::atoi("abc");
// Result: 0
```

**Implementation**:
```cpp
std::stringstream ss(str);
int result;
ss >> result;
return result;
```

**Use Cases**:
- Parsing port numbers
- Parsing mode parameters (e.g., user limit)
- Converting numeric command arguments

**Note**: This is a wrapper around stringstream, providing a more convenient API than std::atoi.

---

## Time Methods

### `std::string getCurrentTime()`

**Purpose**: Gets the current time as a formatted string.

**Returns**: Current time string.

**Format**: Standard ctime format (e.g., "Wed Jan 25 14:30:45 2024")

**Behavior**:
- Gets current time using `time(0)`
- Converts to human-readable format using `ctime()`
- Removes trailing newline character

**Example**:
```cpp
std::string now = Utils::getCurrentTime();
// Example result: "Wed Jan 25 14:30:45 2024"
```

**Implementation**:
```cpp
time_t now = time(0);
char* dt = ctime(&now);
std::string timeStr(dt);
timeStr.erase(timeStr.length() - 1); // Remove newline
return timeStr;
```

**Use Cases**:
- Recording server creation time
- Timestamping events
- Logging

---

## Usage Examples

### Validating User Input
```cpp
// Validate nickname
if (!Utils::isValidNickname("alice123")) {
    client->sendToClient("432 * :Erroneus nickname");
    return;
}

// Validate channel name
if (!Utils::isValidChannelName("#general")) {
    client->sendToClient("403 #general :No such channel");
    return;
}
```

### Parsing Parameters
```cpp
// Split comma-separated channels
std::vector<std::string> channels = Utils::split("#chan1,#chan2,#chan3", ',');

// Convert string to integer
int userLimit = Utils::atoi("10");
```

### String Processing
```cpp
// Case-insensitive command comparison
std::string command = Utils::toUpper(message.getCommand());
if (command == "JOIN") {
    Command_JOIN(server, client, message);
}

// Clean up user input
std::string param = "  value  ";
Utils::trim(param);
// param is now "value"
```

---

## Design Patterns

### Static Utility Class
- All methods are static
- No need to instantiate the class
- Stateless functions
- Thread-safe (no shared state)

### Benefits
1. **Centralization**: All utility functions in one place
2. **Reusability**: Functions can be used across multiple modules
3. **Maintainability**: Easy to find and update utility functions
4. **Testability**: Pure functions are easy to test

---

## Performance Considerations

1. **String Operations**:
   - String copies in `split()` and `toUpper()` create new strings
   - For performance-critical code, consider in-place operations

2. **Validation**:
   - All validation functions are O(n) where n is string length
   - Early return optimization on first invalid character

3. **Memory**:
   - `split()` creates a new vector and strings
   - Consider reusing containers in performance-sensitive code

---

## Dependencies

- `string` - String manipulation
- `vector` - Container for split results
- `sstream` - String stream for parsing
- `ctime` - Time functions
- `algorithm` - std::transform
- `cctype` - Character classification functions (isalpha, isalnum, etc.)

---

## Best Practices

1. **Always Validate Input**: Use validation functions before processing user input
2. **Case-Insensitive Commands**: Use `toUpper()` for command comparison
3. **Clean Input**: Use `trim()` to clean up user-provided strings
4. **Error Handling**: Check return values of validation functions
5. **IRC Compliance**: Follow RFC 1459 specifications implemented in validation functions

---

## Future Enhancements

Possible additions to the Utils class:
1. IPv4/IPv6 validation functions
2. More advanced string manipulation (replace, substring, etc.)
3. Hexadecimal conversion utilities
4. Color code handling for IRC formatting
5. Mask/host pattern matching (for ban lists)

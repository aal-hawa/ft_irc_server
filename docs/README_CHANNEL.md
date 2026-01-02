# Channel Class Documentation

## Overview

The `Channel` class represents an IRC channel - a virtual chat room where multiple users can communicate in real-time. Channels are the primary organizational unit in IRC, allowing users to group together for discussions on specific topics.

## Purpose

The Channel class manages all aspects of channel functionality:
- Channel membership tracking
- Operator privileges management
- Channel modes (invite-only, topic restrictions, password, user limits)
- Invitation system
- Topic management

## Architecture

Channels have a hierarchical structure:
1. **Founder** - The first user to join becomes the channel operator
2. **Operators (@)** - Users with special privileges (kick, ban, change modes)
3. **Regular Members** - Normal users without special privileges
4. **Invited Users** - Users explicitly invited to invite-only channels

## Private Member Variables

| Variable | Type | Description |
|----------|------|-------------|
| `_name` | `std::string` | Channel name (starts with # or &) |
| `_topic` | `std::string` | Channel topic/description |
| `_key` | `std::string` | Channel password (for +k mode) |
| `_members` | `std::map<int, Client*>` | All channel members indexed by file descriptor |
| `_operators` | `std::set<int>` | Set of operator file descriptors |
| `_invited` | `std::set<std::string>` | Set of invited user nicknames |
| `_inviteOnly` | `bool` | Whether channel requires invitation (+i mode) |
| `_topicRestricted` | `bool` | Whether only operators can change topic (+t mode) |
| `_userLimit` | `unsigned int` | Maximum number of members (+l mode, 0 = no limit) |

## Public Methods

### Constructors and Destructors

#### `Channel(const std::string& name, Client* founder)`

**Purpose**: Creates a new channel with the given name and founder.

**Parameters**:
- `name` - The channel name (must start with # or &)
- `founder` - The client creating the channel (becomes first operator)

**Behavior**:
- Initializes channel name and empty topic
- Sets default modes: not invite-only, not topic-restricted, no user limit
- Adds founder to members and makes founder the first operator
- Initializes empty invited set

#### `~Channel()`

**Purpose**: Cleans up channel resources.

**Behavior**:
- Clears all member and operator sets
- Does NOT delete Client objects (managed by Server)
- Prevents memory leaks for channel-owned data

### Getters

#### `std::string getName() const`
Returns the channel name. Example: "#general", "#help", "&private"

#### `std::string getTopic() const`
Returns the current channel topic. Returns empty string if no topic is set.

#### `std::string getKey() const`
Returns the channel password. Returns empty string if no password is set.

#### `bool isInviteOnly() const`
Returns true if the channel is invite-only (+i mode).

#### `bool isTopicRestricted() const`
Returns true if only operators can change the topic (+t mode).

#### `unsigned int getUserLimit() const`
Returns the maximum number of allowed members. Returns 0 if there is no limit.

#### `size_t getMemberCount() const`
Returns the current number of members in the channel.

#### `const std::map<int, Client*>& getMembers() const`
Returns constant reference to the members map. Allows iteration over all channel members.

### Setters

#### `void setTopic(const std::string& topic)`
Sets the channel topic. Permission checking is done by Commands module.

#### `void setKey(const std::string& key)`
Sets or removes the channel password. Empty string removes password.

#### `void setInviteOnly(bool inviteOnly)`
Enables or disables invite-only mode (+i/-i).

#### `void setTopicRestricted(bool restricted)`
Enables or disables topic restriction (+t/-t).

#### `void setUserLimit(unsigned int limit)`
Sets or removes the member limit. 0 = no limit.

### Member Management

#### `void addMember(Client* client)`
Adds a client to the channel. Adds client to members map using file descriptor as key.

#### `void removeMember(Client* client)`
Removes a client from the channel. Also removes client from operators set (if operator).

#### `bool isMember(Client* client) const`
Checks if a client is a member of the channel.

#### `bool isMember(int fd) const`
Checks if a client with a given file descriptor is a member.

### Operator Management

#### `void addOperator(Client* client)`
Grants operator privileges to a client. Only works if client is already a member.

#### `void removeOperator(Client* client)`
Removes operator privileges from a client. Client remains a member.

#### `bool isOperator(Client* client) const`
Checks if a client has operator privileges. Used for permission checks.

### Invite Management

#### `void inviteUser(const std::string& nickname)`
Adds a user to the invited list. Allows user to bypass invite-only restriction.

#### `bool isInvited(const std::string& nickname) const`
Checks if a user has been invited.

#### `void uninviteUser(const std::string& nickname)`
Removes a user from the invited list.

## Channel Modes

### Invite-Only (+i/-i)
- **+i**: Users can only join if invited
- **-i**: Anyone can join (subject to other restrictions)

### Topic-Restricted (+t/-t)
- **+t**: Only operators can change topic
- **-t**: Any member can change topic

### Password-Protected (+k/-k)
- **+k key**: Users must provide correct password to join
- **-k**: No password required

### User Limit (+l/-l)
- **+l N**: Maximum N users allowed in channel
- **-l**: No user limit

## Usage Examples

### Creating a Channel
```cpp
Channel* channel = new Channel("#general", founder);
server->addChannel(channel);
```

### Managing Members
```cpp
channel->addMember(client);
if (channel->isMember(client)) {
    // Client can perform channel operations
}
channel->removeMember(client);
```

### Managing Operators
```cpp
channel->addOperator(client);  // Promote to operator
if (channel->isOperator(client)) {
    // Allow privileged commands
}
channel->removeOperator(client);  // Demote
```

### Channel Modes
```cpp
channel->setInviteOnly(true);
channel->setKey("secretpass");
channel->setUserLimit(10);
channel->setTopicRestricted(true);
```

### Broadcasting to Channel
```cpp
const std::map<int, Client*>& members = channel->getMembers();
for (std::map<int, Client*>::const_iterator it = members.begin();
     it != members.end(); ++it) {
    it->second->sendToClient("Hello everyone!");
}
```

## Channel Lifecycle

1. **Creation**: User JOINs non-existent channel → Channel created → User becomes operator
2. **Normal Operation**: Users JOIN → Members added → Operators can kick, change modes → Messages broadcast
3. **Member Leaves**: User PARTs → Member removed → Operator status removed
4. **Channel Deletion**: Last member leaves → Server removes channel → Channel deleted

## IRC Protocol Compliance

This class follows RFC 1459 and RFC 2812 specifications:
- Channel names start with # or & ✓
- Operators marked with @ prefix ✓
- Channel modes: +i, +k, +l, +t, +o ✓
- Invitation system for +i mode ✓
- Topic management ✓

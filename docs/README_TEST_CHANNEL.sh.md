# test_channel_operations.sh - Channel Operations Test Script

## Overview

This test script validates channel-related functionality including joining channels, sending messages, viewing member lists, managing topics, and leaving channels. It tests interactions between multiple clients.

## Purpose

The test script ensures that:
- Multiple clients can connect simultaneously
- Clients can create and join channels
- Messages are broadcast correctly to channel members
- NAMES command shows all channel members
- Topic can be set and viewed by members
- Clients can leave channels cleanly

## Test Configuration

### Environment Variables
- **PORT**: Server port (default: `6667`)
- **PASSWORD**: Server password (default: `testpass`)

### Test Clients
- **Client 1**: Creates and operates on channels
- **Client 2**: Joins and interacts with Client 1

## Test Cases

### Test 1: User1 Creates and Joins #test

**Purpose**: Verify channel creation and first join.

**Commands** (Client 1):
```
PASS testpass
NICK user1
USER user1 0 * :User One
JOIN #test
```

**Expected Behavior**:
- User1 should become channel operator
- User1 should receive welcome messages
- Channel #test should be created
- User1 should receive JOIN notification

**How It Works**:
```bash
# Create named pipes for bidirectional communication
mkfifo /tmp/client1.pipe
nc -C localhost $PORT < /tmp/client1.pipe > /tmp/client1.out &
CLIENT1=$!
sleep 1

# Authenticate
echo "PASS $PASSWORD" > /tmp/client1.pipe
sleep 1
echo "NICK user1" > /tmp/client1.pipe
sleep 1
echo "USER user1 0 * :User One" > /tmp/client1.pipe
sleep 2

# Join channel
echo "JOIN #test" > /tmp/client1.pipe
sleep 1
```

**Verification**: Channel created, User1 is operator.

---

### Test 2: User2 Joins #test

**Purpose**: Verify multiple clients can join same channel.

**Commands** (Client 2):
```
PASS testpass
NICK user2
USER user2 0 * :User Two
JOIN #test
```

**Expected Behavior**:
- User2 should successfully join channel
- User1 should see User2 join (JOIN message)
- Both clients should see each other in NAMES list
- User2 should NOT be operator (only first member is operator)

**How It Works**:
```bash
mkfifo /tmp/client2.pipe
nc -C localhost $PORT < /tmp/client2.pipe > /tmp/client2.out &
CLIENT2=$!
sleep 1

# Authenticate
echo "PASS $PASSWORD" > /tmp/client2.pipe
sleep 1
echo "NICK user2" > /tmp/client2.pipe
sleep 1
echo "USER user2 0 * :User Two" > /tmp/client2.pipe
sleep 2

# Join channel
echo "JOIN #test" > /tmp/client2.pipe
sleep 1
```

**Verification**: User2 joins, both users can communicate.

---

### Test 3: User1 Sends Message to Channel

**Purpose**: Verify message broadcasting to all channel members.

**Commands** (Client 1):
```
PRIVMSG #test :Hello everyone!
```

**Expected Behavior**:
- User1 sends message to #test
- User2 should receive the message
- User1 should NOT receive own message (no echo)

**How It Works**:
```bash
echo "PRIVMSG #test :Hello everyone!" > /tmp/client1.pipe
sleep 1
```

**Verification**: User2 receives: `:user1!user1@host PRIVMSG #test :Hello everyone!`

---

### Test 4: User2 Checks Names

**Purpose**: Verify NAMES command shows all channel members.

**Commands** (Client 2):
```
NAMES #test
```

**Expected Behavior**:
- Server should send RPL_NAMREPLY (353) with member list
- Operators should be prefixed with `@`
- Server should send RPL_ENDOFNAMES (366)

**Expected Output**:
```
353 user2 = #test :@user1 user2
366 user2 #test :End of /NAMES list
```

**How It Works**:
```bash
echo "NAMES #test" > /tmp/client2.pipe
sleep 1
```

**Verification**: Names list shows both users, user1 has @ prefix.

---

### Test 5: User1 Sets Topic

**Purpose**: Verify topic can be set by operator.

**Commands** (Client 1):
```
TOPIC #test :Welcome to the test channel
```

**Expected Behavior**:
- Topic should be set successfully
- All channel members should be notified
- Server should broadcast TOPIC change

**How It Works**:
```bash
echo "TOPIC #test :Welcome to the test channel" > /tmp/client1.pipe
sleep 1
```

**Verification**: Both clients receive topic change message.

---

### Test 6: User2 Views Topic

**Purpose**: Verify topic viewing functionality.

**Commands** (Client 2):
```
TOPIC #test
```

**Expected Behavior**:
- Server should send RPL_TOPIC (332) with topic text
- Topic should match what was set by User1

**Expected Output**:
```
332 user2 #test :Welcome to the test channel
```

**How It Works**:
```bash
echo "TOPIC #test" > /tmp/client2.pipe
sleep 1
```

**Verification**: User2 receives correct topic.

---

### Test 7: User2 Leaves Channel

**Purpose**: Verify channel leave functionality and cleanup.

**Commands** (Client 2):
```
PART #test :Goodbye!
```

**Expected Behavior**:
- User2 should leave channel
- User1 should receive PART notification with reason
- User2 should no longer be in NAMES list
- Channel should still exist (User1 remains)

**How It Works**:
```bash
echo "PART #test :Goodbye!" > /tmp/client2.pipe
sleep 1
```

**Verification**: User1 sees PART message, User2 not in channel.

---

## Script Structure

### Setup Phase
```bash
#!/bin/bash
PORT=6667
PASSWORD=testpass

echo "=== Testing Channel Operations ==="

# Start server
./ircserv $PORT $PASSWORD &
SERVER_PID=$!
sleep 2

echo "Server started with PID: $SERVER_PID"

# Create temporary pipe files
mkfifo /tmp/client1.pipe
mkfifo /tmp/client2.pipe
```

- Creates named pipes for bidirectional communication
- Starts server
- Prepares for two concurrent client connections

---

### Client Connection Phase
```bash
# Start two clients
nc -C localhost $PORT < /tmp/client1.pipe > /tmp/client1.out &
CLIENT1=$!
nc -C localhost $PORT < /tmp/client2.pipe > /tmp/client2.out &
CLIENT2=$!
sleep 1
```

- Starts netcat for each client
- Stores PIDs for cleanup
- Waits for connections to establish

---

### Cleanup Phase
```bash
# Cleanup
kill $CLIENT1 $CLIENT2 $SERVER_PID 2>/dev/null
wait $CLIENT1 $CLIENT2 $SERVER_PID 2>/dev/null
rm -f /tmp/client1.pipe /tmp/client2.pipe
rm -f /tmp/client1.out /tmp/client2.out

echo ""
echo "=== Channel operations test completed ==="
```

- Terminates all processes
- Removes temporary files
- Waits for clean shutdown

---

## Named Pipe Communication

### Why Named Pipes?

Named pipes (FIFOs) enable bidirectional communication with netcat:
- **Input**: Commands sent to server via pipe → netcat
- **Output**: Server responses stored in output file

### Communication Flow

```
echo "COMMAND" > /tmp/client.pipe
    ↓
[client.pipe] → nc → server
    ↓
server → nc → [client.out]
    ↓
cat /tmp/client.out (to view)
```

---

## Usage

### Running the Test
```bash
# Make script executable
chmod +x scripts/test_channel_operations.sh

# Run the test
./scripts/test_channel_operations.sh
```

### Expected Output
```
=== Testing Channel Operations ===
Server started with PID: 12345

Test 1: User1 joins #test...

Test 2: User2 joins #test...

Test 3: User1 sends message to channel...

Test 4: User2 checks names...

Test 5: User1 sets topic...

Test 6: User2 views topic...

Test 7: User2 leaves channel...

=== Channel operations test completed ===
```

---

## Requirements

### Software Dependencies
- **bash**: Shell interpreter
- **nc**: Netcat for TCP connections
- **mkfifo**: Named pipe creation
- **kill**: Process management

### Server Requirements
- Server binary: `./ircserv`
- Must support channel commands tested
- Port must be available

---

## Verification Steps

### Manual Verification

After running the test, check output files:

```bash
# View Client 1's perspective
cat /tmp/client1.out

# View Client 2's perspective
cat /tmp/client2.out
```

**Expected in Client 1 Output**:
- Welcome messages (001-004)
- JOIN message for #test
- Message from User1
- PART message from User2

**Expected in Client 2 Output**:
- Welcome messages
- JOIN message for #test
- Message from User1
- Topic change notification
- RPL_TOPIC response

---

## Error Handling

### Pipe Creation Fails
- Script may exit with error
- Check if /tmp has write permissions
- Remove existing pipes: `rm -f /tmp/client*.pipe`

### Client Connection Fails
- Check server is running: `ps aux | grep ircserv`
- Verify port is correct
- Check firewall settings

### Process Cleanup Fails
- Manual cleanup required:
```bash
killall nc
pkill ircserv
rm -f /tmp/client*.pipe /tmp/client*.out
```

---

## Debugging

### View Communication in Real-Time

Watch output files while test runs:
```bash
# In separate terminal
watch -n 0.5 'cat /tmp/client1.out'
watch -n 0.5 'cat /tmp/client2.out'
```

### Interactive Testing

Use pipes interactively:
```bash
# Setup
mkfifo /tmp/test.pipe
nc -C localhost 6667 < /tmp/test.pipe

# In another terminal
echo "PASS testpass" > /tmp/test.pipe
echo "NICK test" > /tmp/test.pipe
echo "USER test 0 * :Test" > /tmp/test.pipe
```

### Check Server Logs

If server has logging enabled:
```bash
tail -f /path/to/server.log
```

Or run server in foreground:
```bash
./ircserv 6667 testpass
```

---

## Limitations

1. **Two Clients Only**: Tests only two clients, not more
2. **No Timeouts**: Doesn't test timeout scenarios
3. **Basic Modes**: Doesn't test channel modes (+i, +k, +l, +t)
4. **Output Not Checked**: Doesn't verify exact output
5. **No Error Cases**: Tests only success paths

---

## Extending the Test

### Adding More Clients
```bash
mkfifo /tmp/client3.pipe
nc -C localhost $PORT < /tmp/client3.pipe > /tmp/client3.out &
CLIENT3=$!

echo "PASS $PASSWORD" > /tmp/client3.pipe
echo "NICK user3" > /tmp/client3.pipe
echo "USER user3 0 * :User Three" > /tmp/client3.pipe
echo "JOIN #test" > /tmp/client3.pipe
```

### Testing Channel Modes
```bash
# Set invite-only
echo "MODE #test +i" > /tmp/client1.pipe

# Try to join with new client (should fail)
echo "JOIN #test" > /tmp/client3.pipe
```

### Testing Multiple Channels
```bash
# Join another channel
echo "JOIN #another" > /tmp/client2.pipe

# Send message to both channels
echo "PRIVMSG #test :Message 1" > /tmp/client1.pipe
echo "PRIVMSG #another :Message 2" > /tmp/client1.pipe
```

---

## Integration

This test should be run:
- **After**: `test_basic_commands.sh` (basics work)
- **Before**: `test_operator_commands.sh` (operators test)
- **Before**: `test_stress.sh` (performance test)

Ensures basic channel operations work before testing advanced features.

---

## Troubleshooting

### Issue: Pipes not removed after test
**Cause**: Script didn't complete cleanup
**Solution**: `rm -f /tmp/client*.pipe /tmp/client*.out`

### Issue: "Permission denied" creating pipes
**Cause**: /tmp directory permissions
**Solution**: Check permissions: `ls -ld /tmp` or use different directory

### Issue: Clients don't receive messages
**Cause**: Server not broadcasting correctly
**Solution**: Check Server::broadcastToChannel() implementation

### Issue: PART message not received
**Cause**: Client left before message sent
**Solution**: Increase sleep time after PART command

---

## Best Practices

1. **Cleanup Pipes**: Always remove temporary pipes after use
2. **Adequate Sleeps**: Allow time for server processing between commands
3. **Unique Names**: Use unique pipe names to avoid conflicts
4. **Process Management**: Track all PIDs for proper cleanup
5. **Error Suppression**: Use `2>/dev/null` for expected failures

---

## Exit Codes

Script doesn't set explicit exit codes, but you can check:
```bash
./scripts/test_channel_operations.sh
echo $?
```

- `0`: Script completed (manual output verification needed)
- Non-zero: Script execution failed

---

## Security Notes

1. **Temporary Files**: Pipes created in /tmp (world-writable directory)
2. **Plain Text Passwords**: Password sent in clear text over network
3. **No Encryption**: Netcat sends unencrypted data
4. **PIDs Visible**: Process IDs visible in output

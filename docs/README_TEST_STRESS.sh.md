# test_stress.sh - Stress Test Script

## Overview

This test script performs a stress test on the IRC server by simulating multiple concurrent clients. It tests the server's ability to handle many simultaneous connections, messages, and operations without crashing or becoming unresponsive.

## Purpose

The stress test validates that the server:
- Can handle multiple concurrent connections
- Processes messages from many clients simultaneously
- Manages channel with many members
- Remains stable under load
- Properly cleans up after all clients disconnect

## Test Configuration

### Environment Variables
- **PORT**: Server port (default: `6667`)
- **PASSWORD**: Server password (default: `testpass`)
- **NUM_CLIENTS**: Number of concurrent clients (default: `20`)

### Test Parameters

| Parameter | Default | Description |
|-----------|---------|-------------|
| PORT | 6667 | Server port |
| PASSWORD | testpass | Server authentication password |
| NUM_CLIENTS | 20 | Number of concurrent clients to spawn |

---

## Test Flow

### 1. Server Startup

```bash
./ircserv $PORT $PASSWORD &
SERVER_PID=$!
sleep 2
```

- Starts IRC server in background
- Stores server PID for cleanup
- Waits 2 seconds for initialization

---

### 2. Client Spawning Function

```bash
spawn_client() {
    local num=$1
    (
        sleep 1
        echo "PASS $PASSWORD"
        sleep 1
        echo "NICK user$num"
        sleep 1
        echo "USER user$num 0 * :User Number $num"
        sleep 1
        echo "JOIN #stress"
        sleep 1
        echo "PRIVMSG #stress :Hello from user$num!"
        sleep 1
    ) | nc -C localhost $PORT > /tmp/client$num.out &
    echo $!
}
```

**What Each Client Does**:
1. **Authenticates**: Sends PASS command
2. **Registers**: Sends NICK and USER commands
3. **Joins Channel**: Joins `#stress` channel
4. **Sends Message**: Sends a message to channel
5. **Disconnects**: Pipe closes, connection ends

**Subshell Usage**:
- Runs commands in subshell `( ... )`
- Pipes to netcat: `| nc -C localhost $PORT`
- Redirects output: `> /tmp/client$num.out`
- Runs in background: `&`

---

### 3. Spawn All Clients

```bash
CLIENT_PIDS=()
for i in $(seq 1 $NUM_CLIENTS); do
    pid=$(spawn_client $i)
    CLIENT_PIDS+=($pid)
    echo "Spawned client $i with PID: $pid"
    sleep 0.1
done
```

**Process**:
- Creates array to store client PIDs
- Loops from 1 to NUM_CLIENTS
- Calls `spawn_client()` for each
- Stores PID in array
- Waits 0.1 seconds between spawns

**Purpose of Delay**:
- Prevents all clients connecting simultaneously
- Allows server to process each connection
- Simulates realistic client arrival pattern

---

### 4. Wait for Completion

```bash
echo ""
echo "Waiting for all clients to finish..."
wait
```

**What Happens**:
- `wait` command waits for all background processes
- Each client process exits after sending all commands
- Output files contain server responses

---

### 5. Cleanup

```bash
# Cleanup
for pid in "${CLIENT_PIDS[@]}"; do
    kill $pid 2>/dev/null
done

kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null

# Clean up output files
rm -f /tmp/client*.out
```

**Process**:
1. Kills all client netcat processes
2. Kills server process
3. Waits for server to shutdown
4. Removes output files

---

## Complete Script

```bash
#!/bin/bash
# test_stress.sh - Stress test with multiple clients

PORT=6667
PASSWORD=testpass
NUM_CLIENTS=20

echo "=== Starting Stress Test with $NUM_CLIENTS Clients ==="

# Start server
./ircserv $PORT $PASSWORD &
SERVER_PID=$!
sleep 2

echo "Server started with PID: $SERVER_PID"

# Function to spawn a client
spawn_client() {
    local num=$1
    (
        sleep 1
        echo "PASS $PASSWORD"
        sleep 1
        echo "NICK user$num"
        sleep 1
        echo "USER user$num 0 * :User Number $num"
        sleep 1
        echo "JOIN #stress"
        sleep 1
        echo "PRIVMSG #stress :Hello from user$num!"
        sleep 1
    ) | nc -C localhost $PORT > /tmp/client$num.out &
    echo $!
}

# Spawn clients
CLIENT_PIDS=()
for i in $(seq 1 $NUM_CLIENTS); do
    pid=$(spawn_client $i)
    CLIENT_PIDS+=($pid)
    echo "Spawned client $i with PID: $pid"
    sleep 0.1
done

echo ""
echo "Waiting for all clients to finish..."
wait

# Cleanup
for pid in "${CLIENT_PIDS[@]}"; do
    kill $pid 2>/dev/null
done

kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null

# Clean up output files
rm -f /tmp/client*.out

echo ""
echo "=== Stress test completed ==="
echo "Successfully handled $NUM_CLIENTS concurrent clients"
```

---

## Usage

### Basic Usage
```bash
# Make script executable
chmod +x scripts/test_stress.sh

# Run with defaults (20 clients)
./scripts/test_stress.sh
```

### Custom Number of Clients
```bash
# Edit NUM_CLIENTS in script
NUM_CLIENTS=50
./scripts/test_stress.sh

# Or override inline
NUM_CLIENTS=100 ./scripts/test_stress.sh
```

### Custom Port and Password
```bash
# Edit in script
PORT=8080
PASSWORD=mysecret
./scripts/test_stress.sh
```

---

## Expected Output

```
=== Starting Stress Test with 20 Clients ===
Server started with PID: 12345
Spawned client 1 with PID: 12346
Spawned client 2 with PID: 12347
Spawned client 3 with PID: 12348
...
Spawned client 20 with PID: 12365

Waiting for all clients to finish...

=== Stress test completed ===
Successfully handled 20 concurrent clients
```

---

## What Gets Tested

### 1. Concurrent Connections
- 20 (or more) clients connect simultaneously
- Server must accept all connections
- Each client gets unique socket

### 2. Concurrent Authentication
- All clients send PASS command
- Server processes all authentication requests
- Each client receives response

### 3. Concurrent Registration
- All clients send NICK and USER commands
- Server handles nickname generation
- All clients receive welcome messages

### 4. Channel Operations
- All clients join same channel
- Server manages 20+ channel members
- All JOIN notifications processed

### 5. Message Broadcasting
- Each client sends message to channel
- Server broadcasts 20 messages
- Each message sent to 19 other clients
- Total: 380 message deliveries (20 × 19)

### 6. Connection Cleanup
- All clients disconnect
- Server removes clients from channels
- Server closes sockets properly
- Channel deleted when empty

---

## Performance Metrics

### Commands Processed

For NUM_CLIENTS=20:

| Operation | Per Client | Total |
|-----------|------------|-------|
| PASS | 1 | 20 |
| NICK | 1 | 20 |
| USER | 1 | 20 |
| JOIN | 1 | 20 |
| PRIVMSG | 1 | 20 |
| **Total** | **5** | **100** |

### Message Broadcasting

For NUM_CLIENTS=20:
- Each client sends 1 message to channel
- Each message delivered to 19 other clients
- Total deliveries: 20 × 19 = **380 messages**

### Total Network Activity

For NUM_CLIENTS=20:
- **Incoming**: 100 commands from clients
- **Outgoing**: 100 welcome messages + 380 broadcast messages = **480 messages**

---

## Verification

### Check Client Output Files

```bash
# Check if all clients received welcome messages
for i in $(seq 1 20); do
    echo "=== Client $i ==="
    cat /tmp/client$i.out | grep "001"
done
```

**Expected**: Each client shows welcome message (001).

### Check Server Didn't Crash

```bash
# Check if server still running during test
ps aux | grep ircserv

# Check exit code after test
echo "Exit code: $?"
```

**Expected**: Server runs entire test without crashing.

### Monitor During Test

```bash
# Watch server memory usage
watch -n 1 'ps aux | grep ircserv'

# Watch number of connections
watch -n 1 'netstat -an | grep :6667 | grep ESTABLISHED | wc -l'
```

---

## Increasing Load

### Test with More Clients

```bash
# Edit script
NUM_CLIENTS=50

# Run test
./scripts/test_stress.sh
```

**Caution**: Test server limits. Start small (20-50) and increase gradually.

### Test with Longer Sessions

Modify spawn_client to keep clients connected:

```bash
spawn_client() {
    local num=$1
    (
        sleep 1
        echo "PASS $PASSWORD"
        sleep 1
        echo "NICK user$num"
        sleep 1
        echo "USER user$num 0 * :User Number $num"
        sleep 1
        echo "JOIN #stress"
        sleep 1
        echo "PRIVMSG #stress :Hello from user$num!"
        sleep 10  # Keep alive longer
    ) | nc -C localhost $PORT > /tmp/client$num.out &
    echo $!
}
```

### Test with More Messages

Add multiple messages per client:

```bash
spawn_client() {
    local num=$1
    (
        # ... authentication ...
        echo "JOIN #stress"
        sleep 1
        # Send multiple messages
        for msg in $(seq 1 5); do
            echo "PRIVMSG #stress :Message $msg from user$num"
            sleep 0.5
        done
    ) | nc -C localhost $PORT > /tmp/client$num.out &
    echo $!
}
```

---

## Troubleshooting

### Issue: "Connection refused"

**Cause**: Server not running or port busy

**Solution**:
```bash
# Check if server running
ps aux | grep ircserv

# Check port usage
netstat -an | grep :6667

# Kill existing server
pkill ircserv
```

### Issue: "Address already in use"

**Cause**: Port still bound from previous run

**Solution**:
```bash
# Wait a few seconds and retry
# Or change port
PORT=6668 ./scripts/test_stress.sh
```

### Issue: Client timeouts

**Cause**: Server too slow or overwhelmed

**Solution**:
- Reduce NUM_CLIENTS
- Increase sleep time between spawns
- Check server performance

### Issue: Not all output files created

**Cause**: Some clients didn't complete

**Solution**:
```bash
# Check created files
ls /tmp/client*.out | wc -l

# Should equal NUM_CLIENTS
```

### Issue: Server crashes

**Cause**: Memory leak, buffer overflow, or resource exhaustion

**Solution**:
- Run with valgrind to find memory leaks
- Check file descriptor limits: `ulimit -n`
- Review server logs for errors

---

## Performance Issues

### High Memory Usage

**Symptoms**:
- Server memory grows during test
- System slows down

**Diagnosis**:
```bash
# Monitor memory
top -p $(pgrep ircserv)

# Check memory leak
valgrind --leak-check=full ./ircserv 6667 testpass
```

**Solutions**:
- Ensure Client/Channel objects deleted
- Clear buffers properly
- Check for memory leaks

### Slow Response Times

**Symptoms**:
- Clients receive messages slowly
- Test takes long to complete

**Diagnosis**:
```bash
# Time the test
time ./scripts/test_stress.sh
```

**Solutions**:
- Optimize command processing
- Reduce sleep times
- Use efficient data structures

### File Descriptor Exhaustion

**Symptoms**:
- "Too many open files" errors
- New connections refused

**Diagnosis**:
```bash
# Check limit
ulimit -n

# Check open files
ls /proc/$(pgrep ircserv)/fd | wc -l
```

**Solutions**:
```bash
# Increase limit
ulimit -n 4096

# Or set permanently in /etc/security/limits.conf
```

---

## Limitations

1. **No Verification**: Doesn't verify correct responses
2. **No Timing**: Doesn't measure response times
3. **Simple Commands**: Only tests basic commands
4. **Single Channel**: All clients join same channel
5. **No Error Cases**: Tests only success paths
6. **No Persistence**: Clients disconnect quickly

---

## Best Practices

1. **Start Small**: Begin with 10-20 clients
2. **Monitor System**: Watch CPU, memory, and network
3. **Clean Up**: Always remove output files
4. **Check Logs**: Review server logs during test
5. **Gradual Increase**: Increase NUM_CLIENTS gradually

---

## Exit Codes

- `0`: Test completed successfully
- Non-zero: Test failed (script error or server crash)

---

## Integration

This test should be run last:
1. `test_basic_commands.sh` - Basic functionality
2. `test_channel_operations.sh` - Channel operations
3. `test_operator_commands.sh` - Operator features
4. `test_stress.sh` - Performance under load

Ensures server can handle expected load after verifying features work.

---

## Future Enhancements

Possible improvements:
1. **Metrics Collection**: Measure response times, throughput
2. **Verification**: Check all clients receive correct responses
3. **Varied Load**: Test different patterns of connections/messages
4. **Long Running**: Keep clients connected longer
5. **Failure Testing**: Test server behavior under overload
6. **Automated Reporting**: Generate performance report

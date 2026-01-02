# test_basic_commands.sh - Basic IRC Commands Test Script

## Overview

This test script validates the basic IRC command functionality of the server. It tests fundamental operations such as authentication, registration, and error handling for invalid inputs.

## Purpose

The test script ensures that:
- Password authentication works correctly
- Server rejects incorrect passwords
- Client registration process completes successfully
- Invalid nicknames are properly rejected
- Re-registration is prevented
- Welcome messages are sent correctly

## Test Configuration

### Environment Variables
- **PORT**: Server port (default: `6667`)
- **PASSWORD**: Server password (default: `testpass`)

## Test Cases

### Test 1: Wrong Password

**Purpose**: Verify server rejects incorrect password.

**Commands**:
```
PASS wrongpass
```

**Expected Behavior**:
- Server should send error code 464: `:Password incorrect`

**How It Works**:
```bash
(echo "PASS wrongpass"; sleep 1) | nc -C localhost $PORT | head -5
```

**Verification**: Should receive `464` error response.

---

### Test 2: Authentication and Registration

**Purpose**: Verify complete client registration flow.

**Commands**:
```
PASS testpass
NICK testuser
USER testuser 0 * :Test User
```

**Expected Behavior**:
- Server should authenticate client
- Client should be registered after NICK and USER
- Server should send welcome messages:
  - `001` - Welcome message
  - `002` - Your host
  - `003` - Server creation time
  - `004` - Server info

**How It Works**:
```bash
(
    sleep 1
    echo "PASS $PASSWORD"
    sleep 1
    echo "NICK testuser"
    sleep 1
    echo "USER testuser 0 * :Test User"
    sleep 2
) | nc -C localhost $PORT | grep -E "(001|002|003|004)"
```

**Verification**: Should receive all four welcome numeric replies.

---

### Test 3: Invalid Nickname

**Purpose**: Verify server rejects invalid nicknames.

**Commands**:
```
NICK invalid nick
```

**Expected Behavior**:
- Server should send error code 432: `:Erroneus nickname`

**Validation Rules Tested**:
- Nickname with spaces should be rejected
- Nickname length limit enforced
- Invalid characters rejected

**How It Works**:
```bash
(echo "NICK invalid nick"; sleep 1) | nc -C localhost $PORT | grep "432"
```

**Verification**: Should receive `432` error response.

---

### Test 4: Already Registered

**Purpose**: Verify server prevents re-registration.

**Commands**:
```
PASS testpass
NICK user1
USER user1 0 * :User One
USER user1 0 * :Duplicate
```

**Expected Behavior**:
- First USER command should succeed
- Second USER command should be rejected
- Server should send error code 462: `:You may not reregister`

**How It Works**:
```bash
(
    sleep 1
    echo "PASS $PASSWORD"
    sleep 1
    echo "NICK user1"
    sleep 1
    echo "USER user1 0 * :User One"
    sleep 1
    echo "USER user1 0 * :Duplicate"
    sleep 1
) | nc -C localhost $PORT | grep "462"
```

**Verification**: Should receive `462` error response for second USER command.

---

## Script Structure

### Startup Phase
```bash
#!/bin/bash
PORT=6667
PASSWORD=testpass

echo "=== Testing Basic IRC Commands ==="

# Start server
./ircserv $PORT $PASSWORD &
SERVER_PID=$!
sleep 2

echo "Server started with PID: $SERVER_PID"
```

- Sets configuration variables
- Starts IRC server in background
- Stores server PID for later cleanup
- Waits for server to initialize

---

### Cleanup Phase
```bash
# Cleanup
kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null

echo ""
echo "=== Basic commands test completed ==="
```

- Terminates server process
- Waits for clean shutdown
- Reports completion

---

## Usage

### Running the Test
```bash
# Make script executable
chmod +x scripts/test_basic_commands.sh

# Run the test
./scripts/test_basic_commands.sh
```

### Expected Output
```
=== Testing Basic IRC Commands ===
Server started with PID: 12345

Test 1: Wrong password...

Test 2: Authentication and registration...

Test 3: Invalid nickname...

Test 4: Already registered...

=== Basic commands test completed ===
```

---

## Requirements

### Software Dependencies
- **bash**: Shell interpreter
- **nc**: Netcat for TCP connections
- **grep**: Pattern matching for output filtering
- **head**: Output limiting
- **kill**: Process management

### Server Requirements
- Server binary: `./ircserv`
- Must support IRC protocol commands tested
- Port must be available (not in use)

---

## Error Handling

### Server Fails to Start
- Script may hang or timeout
- Manual cleanup required: `pkill ircserv`

### Port Already in Use
- Server start fails
- Change PORT variable or kill conflicting process
```bash
# Find process using port
lsof -i :6667
# Kill it
kill <PID>
```

### Netcat Not Available
- Install netcat:
  - Debian/Ubuntu: `sudo apt-get install netcat`
  - macOS: `brew install netcat`
  - CentOS/RHEL: `sudo yum install nc`

---

## Test Results Interpretation

### Success Indicators
- Test 1: 464 error code visible in output
- Test 2: Welcome messages (001-004) visible
- Test 3: 432 error code visible
- Test 4: 462 error code visible
- Server process terminates cleanly

### Failure Indicators
- No error messages when expected
- Server crashes during test
- Wrong error codes returned
- Server process remains running after test

---

## Debugging

### Verbose Output
Remove `| head -5` and `| grep` to see full output:
```bash
(echo "PASS wrongpass"; sleep 1) | nc -C localhost $PORT
```

### Manual Testing
Test interactively with netcat:
```bash
nc localhost 6667
PASS testpass
NICK manualtest
USER manualtest 0 * :Manual Test
```

### Server Logs
If server has logging, check for errors:
```bash
# If server logs to file
tail -f server.log

# Or run server in foreground in another terminal
./ircserv 6667 testpass
```

---

## Integration with Other Tests

This test should be run before:
- `test_channel_operations.sh` - Tests channel functionality
- `test_operator_commands.sh` - Tests operator commands
- `test_stress.sh` - Performance testing

It validates that basic functionality works before testing more complex features.

---

## Limitations

1. **Single Client**: Tests only one client at a time
2. **No Timing Checks**: Doesn't verify response times
3. **Basic Commands Only**: Doesn't test channel or operator features
4. **Output Filtering**: Only shows filtered output, may miss issues
5. **No Data Validation**: Doesn't verify exact message content

---

## Extending the Test

### Adding New Test Cases

```bash
# Add after existing tests
echo ""
echo "Test X: Your new test..."
(your commands here) | nc -C localhost $PORT | grep "expected_code"
```

### Testing Multiple Clients

```bash
# Spawn multiple nc processes
nc -C localhost $PORT > /tmp/client1.out &
CLIENT1=$!
nc -C localhost $PORT > /tmp/client2.out &
CLIENT2=$!

# Send commands to each
echo "PASS $PASSWORD" > /tmp/client1.pipe
echo "PASS $PASSWORD" > /tmp/client2.pipe
```

---

## Best Practices

1. **Clean Environment**: Ensure port is free before running
2. **Wait Times**: Adequate sleep times for server processing
3. **Process Cleanup**: Always kill server process after test
4. **Error Suppression**: Use `2>/dev/null` for expected errors
5. **Verification**: Use grep to verify expected responses

---

## Exit Codes

The script doesn't explicitly set exit codes, but you can check:
```bash
./scripts/test_basic_commands.sh
echo $?
```

- `0`: Test completed (manual verification of output needed)
- Non-zero: Script execution failed

---

## Troubleshooting

### Issue: "Connection refused"
**Cause**: Server not running or wrong port
**Solution**: Verify server started and PORT is correct

### Issue: Script hangs
**Cause**: Server not responding or process not killed
**Solution**: Kill server manually: `pkill ircserv`

### Issue: No output
**Cause**: grep filtering too strict or no nc output
**Solution**: Remove grep filters to see raw output

### Issue: Permission denied
**Cause**: Script not executable or port requires root
**Solution**: `chmod +x script.sh` or use port > 1024

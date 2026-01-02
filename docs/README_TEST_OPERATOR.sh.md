# test_operator_commands.sh - Operator Commands Test Script

## Overview

This test script validates operator-related functionality including channel creation, mode changes, user management, kicks, invites, and operator privileges. It tests interactions between three clients with different privilege levels.

## Purpose

The test script ensures that:
- Channel operators have proper privileges
- Channel modes work correctly (+i, +k, +l, +t, +o)
- Operators can kick users from channels
- Invite system works for invite-only channels
- Non-operators cannot perform operator-only actions
- Operator privileges can be granted and revoked

## Test Configuration

### Environment Variables
- **PORT**: Server port (default: `6667`)
- **PASSWORD**: Server password (default: `testpass`)

### Test Clients

| Client | Nickname | Role | Purpose |
|--------|----------|------|---------|
| Client 1 | `operator` | Channel Operator | Performs operator actions |
| Client 2 | `regularuser` | Regular Member | Tests restricted actions |
| Client 3 | `guest` | Guest/Invited | Tests invite system |

## Test Cases

### Test 1: Operator Creates Channel

**Purpose**: Verify first member becomes channel operator.

**Commands** (operator):
```
PASS testpass
NICK operator
USER operator 0 * :Channel Operator
JOIN #testchannel
```

**Expected Behavior**:
- Channel #testchannel is created
- Operator becomes channel operator
- Operator is first and only member

**How It Works**:
```bash
mkfifo /tmp/op_client.pipe
nc -C localhost $PORT < /tmp/op_client.pipe > /tmp/op_client.out &
CLIENT1=$!
sleep 1

echo "PASS $PASSWORD" > /tmp/op_client.pipe
sleep 1
echo "NICK operator" > /tmp/op_client.pipe
sleep 1
echo "USER operator 0 * :Channel Operator" > /tmp/op_client.pipe
sleep 2

echo "JOIN #testchannel" > /tmp/op_client.pipe
sleep 1
```

**Verification**: Operator is operator (verified in later tests).

---

### Test 2: Regular User Joins

**Purpose**: Verify non-operator can join non-restricted channel.

**Commands** (regularuser):
```
PASS testpass
NICK regularuser
USER regularuser 0 * :Regular User
JOIN #testchannel
```

**Expected Behavior**:
- Regular user successfully joins channel
- Regular user is NOT operator
- Both users see JOIN notification

**How It Works**:
```bash
mkfifo /tmp/user_client.pipe
nc -C localhost $PORT < /tmp/user_client.pipe > /tmp/user_client.out &
CLIENT2=$!
sleep 1

echo "PASS $PASSWORD" > /tmp/user_client.pipe
sleep 1
echo "NICK regularuser" > /tmp/user_client.pipe
sleep 1
echo "USER regularuser 0 * :Regular User" > /tmp/user_client.pipe
sleep 2

echo "JOIN #testchannel" > /tmp/user_client.pipe
sleep 1
```

**Verification**: Regular user in channel, not operator.

---

### Test 3: Set Invite-Only Mode (+i)

**Purpose**: Verify operator can set invite-only mode.

**Commands** (operator):
```
MODE #testchannel +i
```

**Expected Behavior**:
- Channel mode changed to invite-only
- Server broadcasts mode change
- All members receive MODE notification

**How It Works**:
```bash
echo "MODE #testchannel +i" > /tmp/op_client.pipe
sleep 1
```

**Verification**: Both clients receive `:server MODE #testchannel +i`

---

### Test 4: Guest Tries to Join (Should Fail)

**Purpose**: Verify invite-only restriction works.

**Commands** (guest):
```
PASS testpass
NICK guest
USER guest 0 * :Guest User
JOIN #testchannel
```

**Expected Behavior**:
- Guest receives error code 473: `:Cannot join channel (+i)`
- Guest does NOT join channel
- Channel members not affected

**How It Works**:
```bash
mkfifo /tmp/guest_client.pipe
nc -C localhost $PORT < /tmp/guest_client.pipe > /tmp/guest_client.out &
CLIENT3=$!
sleep 1

echo "PASS $PASSWORD" > /tmp/guest_client.pipe
sleep 1
echo "NICK guest" > /tmp/guest_client.pipe
sleep 1
echo "USER guest 0 * :Guest User" > /tmp/guest_client.pipe
sleep 2

echo "JOIN #testchannel" > /tmp/guest_client.pipe
sleep 1
```

**Verification**: Guest receives 473 error, not in channel.

---

### Test 5: Operator Invites Guest

**Purpose**: Verify invite system bypasses invite-only restriction.

**Commands** (operator):
```
INVITE guest #testchannel
```

**Expected Behavior**:
- Guest receives INVITE message
- Operator receives RPL_INVITING (341) confirmation
- Guest added to channel's invite list

**How It Works**:
```bash
echo "INVITE guest #testchannel" > /tmp/op_client.pipe
sleep 1
```

**Verification**: Both operator and guest receive invite notifications.

---

### Test 6: Guest Joins After Invite

**Purpose**: Verify invited user can join invite-only channel.

**Commands** (guest):
```
JOIN #testchannel
```

**Expected Behavior**:
- Guest successfully joins channel
- Channel members see JOIN notification
- Guest is not operator (only operator can grant)

**How It Works**:
```bash
echo "JOIN #testchannel" > /tmp/guest_client.pipe
sleep 1
```

**Verification**: Guest in channel, can receive messages.

---

### Test 7: Operator Kicks Regular User

**Purpose**: Verify operator can kick users.

**Commands** (operator):
```
KICK #testchannel regularuser :Testing kick command
```

**Expected Behavior**:
- Regular user removed from channel
- All members receive KICK notification
- Regular user receives message in their output
- Channel still has two members

**How It Works**:
```bash
echo "KICK #testchannel regularuser :Testing kick command" > /tmp/op_client.pipe
sleep 1
```

**Verification**: Regular user no longer in channel, both operator and guest see KICK.

---

### Test 8: Regular User Tries to Kick (Should Fail)

**Purpose**: Verify non-operator cannot kick.

**Commands** (regularuser - rejoined):
```
PASS testpass
NICK kickeduser
USER kickeduser 0 * :Kicked
JOIN #testchannel
KICK #testchannel operator :Cannot kick!
```

**Expected Behavior**:
- Regular user rejoins channel
- Regular user tries to kick operator
- Regular user receives error code 482: `:You're not channel operator`
- Kick does NOT happen

**How It Works**:
```bash
(echo "PASS $PASSWORD"; echo "NICK kickeduser"; echo "USER kickeduser 0 * :Kicked"; sleep 1; echo "JOIN #testchannel"; sleep 1) | nc -C localhost $PORT &
sleep 2
echo "KICK #testchannel operator :Cannot kick!" > /tmp/user_client.pipe
sleep 1
```

**Verification**: Regular user receives 482 error, operator not kicked.

---

### Test 9: Set Channel Key (+k)

**Purpose**: Verify channel password mode.

**Commands** (operator):
```
MODE #testchannel +k secretkey
```

**Expected Behavior**:
- Channel password set to "secretkey"
- Server broadcasts mode change
- Future joins require correct key

**How It Works**:
```bash
echo "MODE #testchannel +k secretkey" > /tmp/op_client.pipe
sleep 1
```

**Verification**: Clients receive `:server MODE #testchannel +k`

---

### Test 10: Set User Limit (+l)

**Purpose**: Verify user limit mode.

**Commands** (operator):
```
MODE #testchannel +l 5
```

**Expected Behavior**:
- Channel member limit set to 5
- Server broadcasts mode change
- Joins after 5 members will be rejected

**How It Works**:
```bash
echo "MODE #testchannel +l 5" > /tmp/op_client.pipe
sleep 1
```

**Verification**: Clients receive `:server MODE #testchannel +l 5`

---

### Test 11: View Channel Modes

**Purpose**: Verify mode viewing functionality.

**Commands** (operator):
```
MODE #testchannel
```

**Expected Behavior**:
- Server sends RPL_CHANNELMODEIS (324)
- Shows all active modes (+i, +k, +l, +t, etc.)

**Expected Output**:
```
324 operator #testchannel +ikl
```

**How It Works**:
```bash
echo "MODE #testchannel" > /tmp/op_client.pipe
sleep 1
```

**Verification**: Operator receives current mode list.

---

### Test 12: Give Operator Privilege (+o)

**Purpose**: Verify operator can grant operator status to others.

**Commands** (operator):
```
MODE #testchannel +o guest
```

**Expected Behavior**:
- Guest becomes channel operator
- Server broadcasts mode change
- Both operator and guest now have operator privileges

**How It Works**:
```bash
echo "MODE #testchannel +o guest" > /tmp/op_client.pipe
sleep 1
```

**Verification**: Guest can now perform operator actions.

---

## Script Structure

### Setup Phase
```bash
#!/bin/bash
PORT=6667
PASSWORD=testpass

echo "=== Testing Operator Commands ==="

# Start server
./ircserv $PORT $PASSWORD &
SERVER_PID=$!
sleep 2

echo "Server started with PID: $SERVER_PID"

# Create temporary pipe files
mkfifo /tmp/op_client.pipe
mkfifo /tmp/user_client.pipe
mkfifo /tmp/guest_client.pipe
```

- Creates three named pipes for three clients
- Starts server
- Waits for initialization

---

### Client Connection Phase
```bash
# Start three clients
nc -C localhost $PORT < /tmp/op_client.pipe > /tmp/op_client.out &
CLIENT1=$!
nc -C localhost $PORT < /tmp/user_client.pipe > /tmp/user_client.out &
CLIENT2=$!
nc -C localhost $PORT < /tmp/guest_client.pipe > /tmp/guest_client.out &
CLIENT3=$!
sleep 1
```

- Starts three netcat processes
- Each client uses separate pipe
- Stores PIDs for cleanup

---

### Cleanup Phase
```bash
# Cleanup
kill $CLIENT1 $CLIENT2 $CLIENT3 $SERVER_PID 2>/dev/null
wait $CLIENT1 $CLIENT2 $CLIENT3 $SERVER_PID 2>/dev/null
rm -f /tmp/op_client.pipe /tmp/user_client.pipe /tmp/guest_client.pipe
rm -f /tmp/op_client.out /tmp/user_client.out /tmp/guest_client.out

echo ""
echo "=== Operator commands test completed ==="
```

- Terminates all processes
- Removes all temporary files
- Waits for clean shutdown

---

## Usage

### Running the Test
```bash
# Make script executable
chmod +x scripts/test_operator_commands.sh

# Run the test
./scripts/test_operator_commands.sh
```

### Expected Output
```
=== Testing Operator Commands ===
Server started with PID: 12345

Test 1: Operator creates #testchannel...

Test 2: Regular user joins channel...

Test 3: Set invite-only mode...

Test 4: Guest tries to join (should fail with 473)...

Test 5: Operator invites guest...

Test 6: Guest joins after invite...

Test 7: Operator kicks regular user...

Test 8: Regular user tries to kick (should fail)...

Test 9: Set channel key...

Test 10: Set user limit...

Test 11: View channel modes...

Test 12: Give operator to guest...

=== Operator commands test completed ===
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
- Must support operator commands
- Must support channel modes: +i, +k, +l, +t, +o
- Port must be available

---

## Verification Steps

### Check Operator Output
```bash
cat /tmp/op_client.out
```

**Expected**:
- Welcome messages
- MODE confirmations
- RPL_INVITING after invite
- RPL_CHANNELMODEIS on mode query

### Check Regular User Output
```bash
cat /tmp/user_client.out
```

**Expected**:
- Welcome messages
- JOIN notifications
- KICK notification
- 482 error on failed kick attempt

### Check Guest Output
```bash
cat /tmp/guest_client.out
```

**Expected**:
- Welcome messages
- 473 error on first join attempt (invite-only)
- INVITE message from operator
- Successful JOIN on second attempt
- Successful JOIN to channel

---

## Error Codes Referenced

| Code | Name | Context |
|------|------|---------|
| 324 | RPL_CHANNELMODEIS | Response to MODE query |
| 341 | RPL_INVITING | Invitation confirmation |
| 473 | ERR_INVITEONLYCHAN | Cannot join (+i) |
| 482 | ERR_CHANOPRIVSNEEDED | Not channel operator |

---

## Troubleshooting

### Issue: Non-operator can kick
**Cause**: Operator privilege check not working
**Solution**: Check `isOperator()` implementation in Channel class

### Issue: Guest can join without invite
**Cause**: Invite-only mode not set or invite check failing
**Solution**: Verify `isInviteOnly()` and `isInvited()` logic

### Issue: Mode changes not broadcast
**Cause**: Broadcast not called after mode change
**Solution**: Check `broadcastToChannel()` calls in MODE handler

### Issue: Operator privilege not granted
**Cause**: `addOperator()` not called or not working
**Solution**: Verify MODE +o implementation

---

## Limitations

1. **Three Clients Only**: Tests limited to three clients
2. **Single Channel**: Only tests one channel
3. **No -o Test**: Doesn't test removing operator privilege
4. **No -k/-l/-i/-t Test**: Doesn't test removing modes
5. **No Persistence**: Modes don't persist after channel empty

---

## Extending the Test

### Test Mode Removal
```bash
# Remove invite-only
echo "MODE #testchannel -i" > /tmp/op_client.pipe

# Remove password
echo "MODE #testchannel -k" > /tmp/op_client.pipe

# Remove user limit
echo "MODE #testchannel -l" > /tmp/op_client.pipe
```

### Test Revoke Operator
```bash
# Remove operator privilege
echo "MODE #testchannel -o guest" > /tmp/op_client.pipe

# Try to kick (should fail)
echo "KICK #testchannel operator :Test" > /tmp/guest_client.pipe
```

### Test Topic Restriction (+t)
```bash
# Set topic-restricted
echo "MODE #testchannel +t" > /tmp/op_client.pipe

# Regular user tries to change topic (should fail)
echo "TOPIC #testchannel :New topic" > /tmp/user_client.pipe
```

### Test Multiple Channels
```bash
# Create second channel
echo "JOIN #other" > /tmp/user_client.pipe

# Make user operator in #other
echo "MODE #other +o regularuser" > /tmp/user_client.pipe
```

---

## Integration

This test should be run:
- **After**: `test_channel_operations.sh` (basic channel ops work)
- **Before**: `test_stress.sh` (performance testing)

Tests advanced operator features after basics verified.

---

## Security Considerations

1. **Privilege Escalation**: Verify non-operators cannot become operators
2. **Mode Changes**: Verify only operators can change modes
3. **Kick Protection**: Verify non-operators cannot kick others
4. **Invite System**: Verify invite-only cannot be bypassed without invite

---

## Best Practices

1. **Test Both Sides**: Verify both success and failure cases
2. **Check Broadcasts**: Ensure all clients receive mode changes
4. **Verify Privileges**: Check operators can do what non-operators can't
5. **Clean PIDs**: Track all client PIDs for proper cleanup
5. **Adequate Delays**: Allow time for server to process each command

---

## Performance Notes

- Three concurrent connections test basic concurrency
- Multiple mode changes test command processing speed
- Named pipes introduce some latency
- Overall test typically completes in 20-30 seconds

---

## Future Enhancements

Possible additions to this test:
1. Test channel banning (if implemented)
2. Test mode combinations (+ikl)
3. Test simultaneous operator actions
4. Test operator persistence across sessions
5. Test operator privilege inheritance

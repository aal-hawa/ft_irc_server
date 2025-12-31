#!/bin/bash
# test_operator_commands.sh - Test operator commands

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

# Start three clients
nc -C localhost $PORT < /tmp/op_client.pipe > /tmp/op_client.out &
CLIENT1=$!
nc -C localhost $PORT < /tmp/user_client.pipe > /tmp/user_client.out &
CLIENT2=$!
nc -C localhost $PORT < /tmp/guest_client.pipe > /tmp/guest_client.out &
CLIENT3=$!
sleep 1

# Authenticate operator
echo "PASS $PASSWORD" > /tmp/op_client.pipe
sleep 1
echo "NICK operator" > /tmp/op_client.pipe
sleep 1
echo "USER operator 0 * :Channel Operator" > /tmp/op_client.pipe
sleep 2

# Authenticate regular user
echo "PASS $PASSWORD" > /tmp/user_client.pipe
sleep 1
echo "NICK regularuser" > /tmp/user_client.pipe
sleep 1
echo "USER regularuser 0 * :Regular User" > /tmp/user_client.pipe
sleep 2

# Authenticate guest
echo "PASS $PASSWORD" > /tmp/guest_client.pipe
sleep 1
echo "NICK guest" > /tmp/guest_client.pipe
sleep 1
echo "USER guest 0 * :Guest User" > /tmp/guest_client.pipe
sleep 2

# Test 1: Operator creates channel
echo ""
echo "Test 1: Operator creates #testchannel..."
echo "JOIN #testchannel" > /tmp/op_client.pipe
sleep 1

# Test 2: Regular user joins
echo ""
echo "Test 2: Regular user joins channel..."
echo "JOIN #testchannel" > /tmp/user_client.pipe
sleep 1

# Test 3: Set channel mode +i (invite-only)
echo ""
echo "Test 3: Set invite-only mode..."
echo "MODE #testchannel +i" > /tmp/op_client.pipe
sleep 1

# Test 4: Guest tries to join (should fail)
echo ""
echo "Test 4: Guest tries to join (should fail with 473)..."
echo "JOIN #testchannel" > /tmp/guest_client.pipe
sleep 1

# Test 5: Operator invites guest
echo ""
echo "Test 5: Operator invites guest..."
echo "INVITE guest #testchannel" > /tmp/op_client.pipe
sleep 1

# Test 6: Guest joins after invite
echo ""
echo "Test 6: Guest joins after invite..."
echo "JOIN #testchannel" > /tmp/guest_client.pipe
sleep 1

# Test 7: Kick regular user
echo ""
echo "Test 7: Operator kicks regular user..."
echo "KICK #testchannel regularuser :Testing kick command" > /tmp/op_client.pipe
sleep 1

# Test 8: Regular user tries to kick (should fail)
echo ""
echo "Test 8: Regular user tries to kick (should fail)..."
(echo "PASS $PASSWORD"; echo "NICK kickeduser"; echo "USER kickeduser 0 * :Kicked"; sleep 1; echo "JOIN #testchannel"; sleep 1) | nc -C localhost $PORT &
sleep 2
echo "KICK #testchannel operator :Cannot kick!" > /tmp/user_client.pipe
sleep 1

# Test 9: Set channel mode +k (key)
echo ""
echo "Test 9: Set channel key..."
echo "MODE #testchannel +k secretkey" > /tmp/op_client.pipe
sleep 1

# Test 10: Set channel mode +l (limit)
echo ""
echo "Test 10: Set user limit..."
echo "MODE #testchannel +l 5" > /tmp/op_client.pipe
sleep 1

# Test 11: View channel modes
echo ""
echo "Test 11: View channel modes..."
echo "MODE #testchannel" > /tmp/op_client.pipe
sleep 1

# Test 12: Give operator privilege
echo ""
echo "Test 12: Give operator to guest..."
echo "MODE #testchannel +o guest" > /tmp/op_client.pipe
sleep 1

# Cleanup
kill $CLIENT1 $CLIENT2 $CLIENT3 $SERVER_PID 2>/dev/null
wait $CLIENT1 $CLIENT2 $CLIENT3 $SERVER_PID 2>/dev/null
rm -f /tmp/op_client.pipe /tmp/user_client.pipe /tmp/guest_client.pipe
rm -f /tmp/op_client.out /tmp/user_client.out /tmp/guest_client.out

echo ""
echo "=== Operator commands test completed ==="

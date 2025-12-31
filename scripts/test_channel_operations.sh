#!/bin/bash
# test_channel_operations.sh - Test channel operations

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

# Start two clients
nc -C localhost $PORT < /tmp/client1.pipe > /tmp/client1.out &
CLIENT1=$!
nc -C localhost $PORT < /tmp/client2.pipe > /tmp/client2.out &
CLIENT2=$!
sleep 1

# Authenticate client 1
echo "PASS $PASSWORD" > /tmp/client1.pipe
sleep 1
echo "NICK user1" > /tmp/client1.pipe
sleep 1
echo "USER user1 0 * :User One" > /tmp/client1.pipe
sleep 2

# Authenticate client 2
echo "PASS $PASSWORD" > /tmp/client2.pipe
sleep 1
echo "NICK user2" > /tmp/client2.pipe
sleep 1
echo "USER user2 0 * :User Two" > /tmp/client2.pipe
sleep 2

# Test 1: User1 creates and joins #test
echo ""
echo "Test 1: User1 joins #test..."
echo "JOIN #test" > /tmp/client1.pipe
sleep 1

# Test 2: User2 joins #test
echo ""
echo "Test 2: User2 joins #test..."
echo "JOIN #test" > /tmp/client2.pipe
sleep 1

# Test 3: Send message to channel
echo ""
echo "Test 3: User1 sends message to channel..."
echo "PRIVMSG #test :Hello everyone!" > /tmp/client1.pipe
sleep 1

# Test 4: View channel names
echo ""
echo "Test 4: User2 checks names..."
echo "NAMES #test" > /tmp/client2.pipe
sleep 1

# Test 5: Set topic
echo ""
echo "Test 5: User1 sets topic..."
echo "TOPIC #test :Welcome to the test channel" > /tmp/client1.pipe
sleep 1

# Test 6: View topic
echo ""
echo "Test 6: User2 views topic..."
echo "TOPIC #test" > /tmp/client2.pipe
sleep 1

# Test 7: User2 leaves channel
echo ""
echo "Test 7: User2 leaves channel..."
echo "PART #test :Goodbye!" > /tmp/client2.pipe
sleep 1

# Cleanup
kill $CLIENT1 $CLIENT2 $SERVER_PID 2>/dev/null
wait $CLIENT1 $CLIENT2 $SERVER_PID 2>/dev/null
rm -f /tmp/client1.pipe /tmp/client2.pipe /tmp/client1.out /tmp/client2.out

echo ""
echo "=== Channel operations test completed ==="

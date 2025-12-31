#!/bin/bash
# test_basic_commands.sh - Test basic IRC commands

PORT=6667
PASSWORD=testpass

echo "=== Testing Basic IRC Commands ==="

# Start server
./ircserv $PORT $PASSWORD &
SERVER_PID=$!
sleep 2

echo "Server started with PID: $SERVER_PID"

# Test 1: Connection with wrong password
echo ""
echo "Test 1: Wrong password..."
(echo "PASS wrongpass"; sleep 1) | nc -C localhost $PORT | head -5

# Test 2: Authentication and registration
echo ""
echo "Test 2: Authentication and registration..."
(
    sleep 1
    echo "PASS $PASSWORD"
    sleep 1
    echo "NICK testuser"
    sleep 1
    echo "USER testuser 0 * :Test User"
    sleep 2
) | nc -C localhost $PORT | grep -E "(001|002|003|004)"

# Test 3: Invalid nickname
echo ""
echo "Test 3: Invalid nickname..."
(echo "NICK invalid nick"; sleep 1) | nc -C localhost $PORT | grep "432"

# Test 4: Already registered
echo ""
echo "Test 4: Already registered..."
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

# Cleanup
kill $SERVER_PID 2>/dev/null
wait $SERVER_PID 2>/dev/null

echo ""
echo "=== Basic commands test completed ==="

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

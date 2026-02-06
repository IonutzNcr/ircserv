#!/bin/bash
# Minimal test: send PASS, NICK and USER to the IRC server using nc (netcat)
# Usage:
#   chmod +x scripts/test_nick.sh
#   ./scripts/test_nick.sh

HOST=localhost
PORT=4444

echo "Connecting to $HOST:$PORT..."
{
  printf "PASS caca\r\n"
  sleep 0.5
  printf "NICK toto\r\n"
  sleep 0.5
  printf "USER arnaud 0 * :Arnaud\r\n"
  # keep connection open for a few seconds to receive server responses
  sleep 2
} | nc "$HOST" "$PORT"

RC=$?
echo "nc exit code: $RC"

# If your nc supports -q, you might prefer: | nc -q 1 "$HOST" "$PORT"

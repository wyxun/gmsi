#!/bin/bash
# Start OpenOCD RTT
make rtt > openocd_debug.log 2>&1 &
OCD_PID=$!
sleep 2

# Start RTT Client
nc localhost 9090 > rtt_debug.log 2>&1 &
NC_PID=$!

# Send Reset via Telnet
echo "reset" | nc localhost 4444
sleep 1
# Send Resume just in case
echo "resume" | nc localhost 4444

# Wait for logs
sleep 5

# Show logs
echo "--- RTT Log ---"
cat rtt_debug.log

# Cleanup
kill $OCD_PID $NC_PID

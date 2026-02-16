#!/bin/bash
# Generate random test file
dd if=/dev/urandom of=test.bin bs=1024 count=4 status=none

# Configure Serial Port
stty -F /dev/ttyUSB0 115200 raw -echo -hupcl -crtscts

# Start RTT logging in background
nc localhost 9090 > rtt_transfer.log 2>&1 &
RTT_PID=$!

echo "Starting Ymodem Transfer..."
# Run sb with timeout 30s
timeout 30s sb --ymodem -vv test_app_0x08008000.bin < /dev/ttyUSB0 > /dev/ttyUSB0 2> sb_output.log

SB_EXIT=$?
echo "sb exit code: $SB_EXIT"

# Kill nc
kill $RTT_PID

# Show logs
echo "--- sb output ---"
cat sb_output.log
echo "--- RTT Log (Tail) ---"
tail -n 20 rtt_transfer.log

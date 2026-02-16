#!/bin/bash
echo "--- Cleanup ---"
pkill -9 openocd
pkill -9 tio
pkill -9 minicom
pkill -9 picocom
pkill -9 sb
fuser -k /dev/ttyUSB0
fuser -k /dev/ttyACM0

echo "--- Flashing ---"
make CHIP=at32f4 flash
if [ $? -ne 0 ]; then
    echo "Flash FAILED. Hardware might need re-plug."
else
    echo "Flash SUCCESS."
fi

echo "--- Starting RTT Monitor (Background) ---"
make CHIP=at32f4 rtt > openocd_rtt.log 2>&1 &
sleep 3

echo "--- Ymodem Transfer Test (test.bin) ---"
dd if=/dev/urandom of=test.bin bs=1024 count=4 status=none
stty -F /dev/ttyUSB0 115200 raw -echo -hupcl -crtscts
timeout 30s sb --ymodem -vv test.bin < /dev/ttyUSB0 > /dev/ttyUSB0 2> sb_debug.log

echo "--- Transfer Result ---"
cat sb_debug.log
echo "--- RTT Snapshot ---"
nc localhost 9090 | head -n 30

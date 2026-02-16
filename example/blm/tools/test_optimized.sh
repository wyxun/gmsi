#!/bin/bash
# Quick test script for optimized bootloader

echo "=== Optimized Bootloader Test ==="
echo ""

# Check if binary exists
if [ ! -f "build/at32f4/blm.bin" ]; then
    echo "ERROR: Bootloader binary not found!"
    echo "Run: make CHIP=at32f4"
    exit 1
fi

# Check size
SIZE=$(stat -c%s "build/at32f4/blm.bin")
echo "1. Bootloader Size Check:"
echo "   Size: $SIZE bytes"

if [ $SIZE -le 16384 ]; then
    echo "   ✓ Within 16KB limit ($(($SIZE * 100 / 16384))% used)"
else
    echo "   ✗ EXCEEDS 16KB limit!"
    exit 1
fi

# Get detailed size
echo ""
echo "2. Detailed Size Analysis:"
~/llvm_for_arm/bin/llvm-size build/at32f4/blm.elf

# Calculate totals
TEXT=$(~/llvm_for_arm/bin/llvm-size build/at32f4/blm.elf | awk 'NR==2 {print $1}')
DATA=$(~/llvm_for_arm/bin/llvm-size build/at32f4/blm.elf | awk 'NR==2 {print $2}')
BSS=$(~/llvm_for_arm/bin/llvm-size build/at32f4/blm.elf | awk 'NR==2 {print $3}')
FLASH=$((TEXT + DATA))

echo ""
echo "3. Resource Usage:"
echo "   Flash: $FLASH bytes (text + data)"
echo "   RAM:   $BSS bytes (bss)"
echo ""

# Verify vector table
echo "4. Binary Verification:"
python3 << 'EOF'
import struct
with open('build/at32f4/blm.bin', 'rb') as f:
    data = f.read(8)
sp = struct.unpack('<I', data[0:4])[0]
rh = struct.unpack('<I', data[4:8])[0]

print(f"   Stack: 0x{sp:08X} - {'✓' if (sp & 0xFFF00000) == 0x20000000 else '✗'}")
print(f"   Reset: 0x{rh:08X} - {'✓' if (rh & 0xFF000000) == 0x08000000 else '✗'}")
EOF

echo ""
echo "5. Test Instructions:"
echo ""
echo "   Method A: Flash and test with test_app"
echo "   ----------------------------------------"
echo "   make flash CHIP=at32f4"
echo "   ./test_transfer.sh test_app_0x08008000.bin"
echo ""
echo "   Method B: Flash via OpenOCD"
echo "   ----------------------------"
echo "   openocd -f interface/cmsis-dap.cfg -c \"cmsis_dap_backend hid\" \\"
echo "           -f target/at32f407xx.cfg \\"
echo "           -c \"program build/at32f4/blm.bin verify reset exit\""
echo ""
echo "6. Expected Behavior:"
echo "   - Bootloader: LED slow blink (500ms)"
echo "   - Test APP: LED fast blink (200ms)"
echo ""
echo "✓ Optimized bootloader ready for testing!"

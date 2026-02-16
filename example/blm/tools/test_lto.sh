#!/bin/bash
# Quick test: Enable LTO and measure size reduction

echo "=== LTO Optimization Test ==="
echo ""

# Backup current makefile
if [ ! -f "makefile.pre-lto" ]; then
    cp makefile makefile.pre-lto
    echo "✓ Makefile backed up to makefile.pre-lto"
fi

# Get current size
if [ -f "build/at32f4/blm.elf" ]; then
    BEFORE_TEXT=$(~/llvm_for_arm/bin/llvm-size build/at32f4/blm.elf | awk 'NR==2 {print $1}')
    BEFORE_DATA=$(~/llvm_for_arm/bin/llvm-size build/at32f4/blm.elf | awk 'NR==2 {print $2}')
    BEFORE_FLASH=$((BEFORE_TEXT + BEFORE_DATA))
    echo "Before LTO:"
    echo "  Flash: $BEFORE_FLASH bytes"
    echo ""
else
    echo "No existing build found, building first..."
    make clean && make CHIP=at32f4 >/dev/null 2>&1
    BEFORE_TEXT=$(~/llvm_for_arm/bin/llvm-size build/at32f4/blm.elf | awk 'NR==2 {print $1}')
    BEFORE_DATA=$(~/llvm_for_arm/bin/llvm-size build/at32f4/blm.elf | awk 'NR==2 {print $2}')
    BEFORE_FLASH=$((BEFORE_TEXT + BEFORE_DATA))
    echo "Before LTO:"
    echo "  Flash: $BEFORE_FLASH bytes"
    echo ""
fi

# Add LTO flags
echo "Adding LTO flags to makefile..."
if ! grep -q "^CFLAGS.*-flto" makefile; then
    # Find the CFLAGS line and add -flto
    sed -i '/^CFLAGS = .*$(OPT)/s/$/ -flto/' makefile
fi

if ! grep -q "^LDFLAGS.*-flto" makefile; then
    # Find the LDFLAGS line and add -flto
    sed -i '/^LDFLAGS = .*$(CPU_FLAGS)/s/$/ -flto/' makefile
fi

echo "✓ LTO flags added"
echo ""

# Rebuild
echo "Rebuilding with LTO (this may take longer)..."
make clean >/dev/null 2>&1
if make CHIP=at32f4 2>&1 | tail -5; then
    echo ""
    echo "✓ Build successful"
    echo ""

    # Get new size
    AFTER_TEXT=$(~/llvm_for_arm/bin/llvm-size build/at32f4/blm.elf | awk 'NR==2 {print $1}')
    AFTER_DATA=$(~/llvm_for_arm/bin/llvm-size build/at32f4/blm.elf | awk 'NR==2 {print $2}')
    AFTER_FLASH=$((AFTER_TEXT + AFTER_DATA))

    echo "After LTO:"
    echo "  Flash: $AFTER_FLASH bytes"
    echo ""

    # Calculate savings
    SAVED=$((BEFORE_FLASH - AFTER_FLASH))
    PERCENT=$((SAVED * 100 / BEFORE_FLASH))

    echo "=== Results ==="
    echo "  Saved: $SAVED bytes ($PERCENT%)"
    echo ""

    if [ $AFTER_FLASH -lt 16384 ]; then
        MARGIN=$((16384 - AFTER_FLASH))
        echo "  ✓ Within 16KB target"
        echo "  ✓ Margin: $MARGIN bytes"
    else
        OVER=$((AFTER_FLASH - 16384))
        echo "  ✗ Exceeds 16KB by $OVER bytes"
    fi
    echo ""

    echo "To keep LTO:"
    echo "  # LTO is now enabled in makefile"
    echo ""
    echo "To revert:"
    echo "  mv makefile.pre-lto makefile"
    echo "  make clean && make CHIP=at32f4"
else
    echo ""
    echo "✗ Build failed with LTO"
    echo ""
    echo "Reverting makefile..."
    mv makefile.pre-lto makefile
    exit 1
fi

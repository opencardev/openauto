#!/bin/bash
# OpenAuto Package Validation Script

set -e

PACKAGE_FILE="$1"

if [ -z "$PACKAGE_FILE" ]; then
    echo "Usage: $0 <package-file.deb>"
    exit 1
fi

if [ ! -f "$PACKAGE_FILE" ]; then
    echo "Error: Package file not found: $PACKAGE_FILE"
    exit 1
fi

echo "🔍 Validating OpenAuto Package: $PACKAGE_FILE"
echo ""

# Basic package info
echo "📦 Package Information:"
dpkg-deb -I "$PACKAGE_FILE" | grep -E "(Package|Version|Architecture|Depends|Description)"
echo ""

# Package contents validation
echo "📋 Validating Package Contents:"

# Check essential files
REQUIRED_FILES=(
    "./opt/openauto/autoapp"
    "./opt/openauto/btservice"
    "./etc/openauto/openauto.conf"
    "./etc/systemd/system/openauto.service"
    "./etc/systemd/system/openauto-btservice.service"
    "./etc/udev/rules.d/99-openauto.rules"
    "./opt/openauto/scripts/openauto-setup.sh"
)

MISSING_FILES=0

for file in "${REQUIRED_FILES[@]}"; do
    if dpkg-deb -c "$PACKAGE_FILE" | grep -q "$file"; then
        echo "   ✅ $file"
    else
        echo "   ❌ $file (MISSING)"
        MISSING_FILES=$((MISSING_FILES + 1))
    fi
done

echo ""

# Check file permissions in package
echo "🔐 Checking File Permissions:"
dpkg-deb -c "$PACKAGE_FILE" | grep -E "(autoapp|btservice|\.sh)" | while read line; do
    if echo "$line" | grep -q "rwxr-xr-x"; then
        echo "   ✅ $(echo "$line" | awk '{print $6}') - executable"
    else
        echo "   ⚠️  $(echo "$line" | awk '{print $6}') - may not be executable"
    fi
done

echo ""

# Package size check
PACKAGE_SIZE=$(du -h "$PACKAGE_FILE" | cut -f1)
echo "📊 Package Size: $PACKAGE_SIZE"

# Dependencies check
echo ""
echo "🔗 Dependencies:"
dpkg-deb -I "$PACKAGE_FILE" | grep "Depends:" | sed 's/Depends: //' | tr ',' '\n' | while read dep; do
    dep=$(echo "$dep" | xargs)  # trim whitespace
    if [ -n "$dep" ]; then
        echo "   • $dep"
    fi
done

echo ""

# Summary
if [ "$MISSING_FILES" -eq 0 ]; then
    echo "✅ Package validation PASSED"
    echo ""
    echo "🚀 Ready to install:"
    echo "   sudo apt install $PACKAGE_FILE"
    exit 0
else
    echo "❌ Package validation FAILED"
    echo "   Missing $MISSING_FILES essential files"
    exit 1
fi

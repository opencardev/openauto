#!/bin/bash
# PNG Profile Checker Script
# This script checks PNG files for problematic iCCP profiles

echo "🔧 PNG Profile Checker for OpenAuto"
echo "===================================="

ASSETS_DIR="${1:-assets}"

if [ ! -d "$ASSETS_DIR" ]; then
    echo "❌ Assets directory '$ASSETS_DIR' not found!"
    echo "Usage: $0 [assets_directory]"
    exit 1
fi

echo "📁 Checking PNG files in: $ASSETS_DIR"
echo ""

# Check if required tools are available
if ! command -v pngcheck >/dev/null 2>&1; then
    echo "⚠️  pngcheck not found. Install with: sudo apt-get install pngcheck"
    exit 1
fi

PNG_FILES=$(find "$ASSETS_DIR" -name "*.png" 2>/dev/null)

if [ -z "$PNG_FILES" ]; then
    echo "ℹ️  No PNG files found in $ASSETS_DIR"
    exit 0
fi

PROBLEMATIC_COUNT=0
TOTAL_COUNT=0

echo "🔍 Scanning PNG files..."
echo ""

for PNG_FILE in $PNG_FILES; do
    FILENAME=$(basename "$PNG_FILE")
    TOTAL_COUNT=$((TOTAL_COUNT + 1))
    
    # Check if file has iCCP profile
    PNGCHECK_OUTPUT=$(pngcheck -v "$PNG_FILE" 2>/dev/null)
    
    if echo "$PNGCHECK_OUTPUT" | grep -q "iCCP"; then
        PROBLEMATIC_COUNT=$((PROBLEMATIC_COUNT + 1))
        echo "🚨 PROBLEMATIC: $FILENAME"
        echo "   📁 Path: $PNG_FILE"
        echo "   ⚠️  Contains incorrect sRGB profile"
        echo "   💡 Will cause: 'libpng warning: iCCP: known incorrect sRGB profile'"
        
        # Show how to fix manually
        echo "   🔧 To fix manually:"
        if command -v pngcrush >/dev/null 2>&1; then
            echo "      pngcrush -rem iCCP \"$PNG_FILE\" \"$PNG_FILE.fixed\""
        fi
        if command -v convert >/dev/null 2>&1; then
            echo "      convert \"$PNG_FILE\" -strip \"$PNG_FILE.fixed\""
        fi
        echo ""
    else
        echo "✅ CLEAN: $FILENAME"
    fi
done

echo ""
echo "📊 Summary:"
echo "   Total PNG files: $TOTAL_COUNT"
echo "   Problematic files: $PROBLEMATIC_COUNT"
echo "   Clean files: $((TOTAL_COUNT - PROBLEMATIC_COUNT))"

if [ $PROBLEMATIC_COUNT -gt 0 ]; then
    echo ""
    echo "⚠️  Found $PROBLEMATIC_COUNT problematic PNG files!"
    echo "💡 These will be automatically fixed during CMake configuration."
    echo "🔧 Or install tools manually: sudo apt-get install pngcrush imagemagick"
    exit 1
else
    echo ""
    echo "✅ All PNG files are clean!"
    exit 0
fi
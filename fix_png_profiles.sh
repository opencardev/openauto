#!/bin/bash

# Script to fix libpng iCCP profile warnings
# This script removes problematic color profiles from PNG files

echo "🔧 Fixing PNG color profiles..."

# Create backup directory
mkdir -p assets_backup
echo "📁 Created backup directory: assets_backup"

# Copy all PNG files to backup
cp assets/*.png assets_backup/
echo "💾 Backed up all PNG files"

# Counter for processed files
fixed_count=0
total_count=0

# Process each PNG file
for png_file in assets/*.png; do
    if [[ -f "$png_file" ]]; then
        filename=$(basename "$png_file")
        total_count=$((total_count + 1))
        
        echo "🔍 Processing: $filename"
        
        # Check if the file has an iCCP profile
        if pngcheck -v "$png_file" 2>/dev/null | grep -q "iCCP"; then
            echo "  ⚠️  Found problematic iCCP profile in: $filename"
            echo "      📄 File path: $png_file"
            echo "      🔧 This file will cause libpng warnings at runtime"
            
            # Method 1: Use pngcrush to remove iCCP profile
            if pngcrush -rem iCCP "$png_file" "${png_file}.tmp" >/dev/null 2>&1; then
                mv "${png_file}.tmp" "$png_file"
                echo "  ✅ Fixed iCCP profile using pngcrush"
                fixed_count=$((fixed_count + 1))
            else
                # Method 2: Use ImageMagick as fallback
                echo "  🔄 Trying ImageMagick fallback..."
                if convert "$png_file" -strip "${png_file}.tmp" 2>/dev/null; then
                    mv "${png_file}.tmp" "$png_file"
                    echo "  ✅ Fixed using ImageMagick"
                    fixed_count=$((fixed_count + 1))
                else
                    echo "  ❌ Failed to fix $filename"
                    rm -f "${png_file}.tmp"
                fi
            fi
        else
            echo "  ✅ No iCCP profile found - OK"
        fi
    fi
done

echo ""
echo "🎉 PNG Profile Fix Complete!"
echo "📊 Summary:"
echo "   Total files processed: $total_count"
echo "   Files fixed: $fixed_count"
echo "   Backup location: assets_backup/"
echo ""
echo "ℹ️  You can now restart autoapp to see if the libpng warning is gone."

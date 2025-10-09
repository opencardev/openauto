#!/bin/bash
# Fix all logging syntax errors in the openauto project

echo "Fixing logging syntax errors across all C++ files..."

# Find all .cpp files and fix them
find src -name "*.cpp" -exec sed -i \
    -e 's/Priority: ";/Priority: "/g' \
    -e 's/Channel ";/Channel "/g' \
    -e 's/Channel Id: ";/Channel Id: "/g' \
    -e 's/Status determined: ";/Status determined: "/g' \
    -e 's/Codec: ";/Codec: "/g' \
    -e 's/anc: ";/anc: "/g' \
    -e 's/session: ";/session: "/g' \
    -e 's/what();/what()/g' \
    -e 's/\.str())$/\.str());/g' \
    {} \;

echo "Fixed logging syntax in all source files."

# Now try to build and see if there are any remaining errors
echo "Testing build..."
cd build && make -j$(nproc) 2>&1 | head -20
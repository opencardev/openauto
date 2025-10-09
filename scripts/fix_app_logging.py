#!/usr/bin/env python3

import re

# Read the file
with open('/home/pi/openauto_ori/src/autoapp/App.cpp', 'r') as f:
    content = f.read()

# Fix pattern: ([message]).str())" -> "message"
pattern = r'OPENAUTO_LOG_ERROR\(([^,]+),\s*\("\(([^)]+)\)\.str\(\)"\);'
replacement = r'OPENAUTO_LOG_ERROR(\1, "\2");'

fixed_content = re.sub(pattern, replacement, content)

# Also fix pattern where there's no quotes: ([message]).str())" -> "message"
pattern2 = r'OPENAUTO_LOG_ERROR\(([^,]+),\s*\(\[([^\]]+)\]\)\.str\(\)\)";'
replacement2 = r'OPENAUTO_LOG_ERROR(\1, "[\2]");'

fixed_content = re.sub(pattern2, replacement2, fixed_content)

# Fix remaining malformed patterns
patterns_to_fix = [
    (r'\("\[([^\]]+)\]\)\.str\(\)\)";', r';"[\1]";'),
    (r'\(([^)]+)\)\.str\(\)\)";', r'"\1";'),
]

for pattern, replacement in patterns_to_fix:
    fixed_content = re.sub(pattern, replacement, fixed_content)

# Write the fixed content back
with open('/home/pi/openauto_ori/src/autoapp/App.cpp', 'w') as f:
    f.write(fixed_content)

print("Fixed App.cpp logging statements")
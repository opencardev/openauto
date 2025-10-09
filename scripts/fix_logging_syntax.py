#!/usr/bin/env python3
"""
Fix syntax errors in logging statements caused by migration script.
This script fixes the pattern where semicolons were incorrectly placed 
in the middle of stringstream operations.
"""

import re
import os
import glob

def fix_logging_syntax(file_path):
    """Fix broken logging syntax in a single file."""
    with open(file_path, 'r') as f:
        content = f.read()
    
    original_content = content
    
    # Pattern to match broken logging statements:
    # OPENAUTO_LOG_xxx(category, (std::stringstream() << "text" << var;
    #                              << more_vars).str())
    
    # First, find all multi-line broken logging statements
    # This regex matches the pattern where we have a semicolon in the middle
    pattern = r'(OPENAUTO_LOG_\w+\([^,]+,\s*\(std::stringstream\(\)[^;]*);(\s*<<[^)]*\)\.str\(\)\))'
    
    def fix_match(match):
        first_part = match.group(1)
        second_part = match.group(2)
        # Replace the semicolon with nothing and add semicolon at the end
        return first_part + second_part + ';'
    
    # Apply the fix
    fixed_content = re.sub(pattern, fix_match, content, flags=re.MULTILINE | re.DOTALL)
    
    if fixed_content != original_content:
        print(f"Fixed logging syntax in {file_path}")
        with open(file_path, 'w') as f:
            f.write(fixed_content)
        return True
    return False

def main():
    """Fix logging syntax in all C++ source files."""
    source_patterns = [
        "src/**/*.cpp",
        "include/**/*.hpp"
    ]
    
    files_fixed = 0
    
    for pattern in source_patterns:
        for file_path in glob.glob(pattern, recursive=True):
            if fix_logging_syntax(file_path):
                files_fixed += 1
    
    print(f"Fixed logging syntax in {files_fixed} files.")

if __name__ == "__main__":
    main()
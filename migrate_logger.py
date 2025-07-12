#!/usr/bin/env python3
"""
OpenAuto Logger Migration Script

This script migrates all OPENAUTO_LOG calls to the new modern logger.
It performs the following transformations:
1. Replaces #include <f1x/openauto/Common/Log.hpp> with #include <modern/Logger.hpp>
2. Converts OPENAUTO_LOG(severity) calls to new LOG_* macros
3. Maps log severities to appropriate categories
4. Preserves original message context where possible
"""

import os
import re
import sys
import argparse
from pathlib import Path

# Mapping of log severities to new log levels
SEVERITY_MAPPING = {
    'trace': 'LOG_TRACE',
    'debug': 'LOG_DEBUG', 
    'info': 'LOG_INFO',
    'warning': 'LOG_WARN',
    'error': 'LOG_ERROR',
    'fatal': 'LOG_FATAL'
}

# Category mapping based on file patterns and context
CATEGORY_MAPPING = {
    'btservice': 'BLUETOOTH',
    'Service': 'ANDROID_AUTO',
    'UI': 'UI',
    'Camera': 'CAMERA',
    'Video': 'VIDEO',
    'Audio': 'AUDIO',
    'Media': 'AUDIO',
    'Sensor': 'SYSTEM',
    'Navigation': 'ANDROID_AUTO',
    'Radio': 'AUDIO',
    'Phone': 'ANDROID_AUTO',
    'Wifi': 'NETWORK',
    'Network': 'NETWORK',
    'Connection': 'NETWORK',
    'Configuration': 'CONFIG',
    'Factory': 'SYSTEM'
}

def determine_category(file_path, context=''):
    """Determine the appropriate log category based on file path and context"""
    file_str = str(file_path).lower()
    
    # Check for specific patterns in file path
    for pattern, category in CATEGORY_MAPPING.items():
        if pattern.lower() in file_str:
            return category
    
    # Check context for hints
    context_lower = context.lower()
    for pattern, category in CATEGORY_MAPPING.items():
        if pattern.lower() in context_lower:
            return category
    
    # Default category
    return 'GENERAL'

def extract_message_from_stream(stream_content):
    """Extract the actual message from a log stream"""
    # Remove common prefixes like [ClassName::method]
    message = re.sub(r'^\[[\w:]+\]\s*', '', stream_content)
    
    # Remove quotes if the entire message is quoted
    if message.startswith('"') and message.endswith('"'):
        message = message[1:-1]
    
    return message.strip()

def convert_log_call(match, file_path):
    """Convert an OPENAUTO_LOG call to the new logger format"""
    severity = match.group(1)
    content = match.group(2)
    
    # Map severity to new macro
    if severity not in SEVERITY_MAPPING:
        print(f"Warning: Unknown severity '{severity}' in {file_path}")
        return match.group(0)  # Return original if unknown
    
    new_macro = SEVERITY_MAPPING[severity]
    category = determine_category(file_path, content)
    
    # Extract message from stream content
    message = extract_message_from_stream(content)
    
    # Create the new log call
    return f'{new_macro}({category}, "{message}");'

def process_file(file_path):
    """Process a single file to migrate logging calls"""
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            content = f.read()
    except Exception as e:
        print(f"Error reading {file_path}: {e}")
        return False
    
    original_content = content
    modified = False
    
    # Replace include statement
    if '#include <f1x/openauto/Common/Log.hpp>' in content:
        content = content.replace(
            '#include <f1x/openauto/Common/Log.hpp>',
            '#include <modern/Logger.hpp>'
        )
        modified = True
    
    # Convert OPENAUTO_LOG calls
    # Pattern matches: OPENAUTO_LOG(severity) << "message content";
    pattern = r'OPENAUTO_LOG\((\w+)\)\s*<<\s*([^;]+);'
    
    def replace_func(match):
        nonlocal modified
        modified = True
        return convert_log_call(match, file_path)
    
    content = re.sub(pattern, replace_func, content)
    
    # Write back if modified
    if modified:
        try:
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(content)
            print(f"Migrated: {file_path}")
            return True
        except Exception as e:
            print(f"Error writing {file_path}: {e}")
            return False
    
    return False

def find_cpp_files(root_dir):
    """Find all C++ source files in the directory tree"""
    cpp_files = []
    for root, dirs, files in os.walk(root_dir):
        # Skip certain directories
        if any(skip_dir in root for skip_dir in ['build', '.git', 'cmake']):
            continue
        
        for file in files:
            if file.endswith(('.cpp', '.c', '.cc', '.cxx')):
                cpp_files.append(os.path.join(root, file))
    
    return cpp_files

def main():
    parser = argparse.ArgumentParser(description='Migrate OpenAuto logging to modern logger')
    parser.add_argument('directory', help='Root directory to process')
    parser.add_argument('--dry-run', action='store_true', help='Show what would be changed without modifying files')
    parser.add_argument('--verbose', action='store_true', help='Verbose output')
    
    args = parser.parse_args()
    
    if not os.path.isdir(args.directory):
        print(f"Error: {args.directory} is not a valid directory")
        return 1
    
    cpp_files = find_cpp_files(args.directory)
    
    if args.verbose:
        print(f"Found {len(cpp_files)} C++ files to process")
    
    migrated_count = 0
    
    for file_path in cpp_files:
        if args.dry_run:
            # In dry run mode, just check what would be changed
            try:
                with open(file_path, 'r', encoding='utf-8') as f:
                    content = f.read()
                
                if ('OPENAUTO_LOG' in content or 
                    '#include <f1x/openauto/Common/Log.hpp>' in content):
                    print(f"Would migrate: {file_path}")
                    migrated_count += 1
            except Exception as e:
                print(f"Error reading {file_path}: {e}")
        else:
            if process_file(file_path):
                migrated_count += 1
    
    print(f"\n{'Would migrate' if args.dry_run else 'Migrated'} {migrated_count} files")
    
    if not args.dry_run and migrated_count > 0:
        print("\nMigration complete! Don't forget to:")
        print("1. Add #include <modern/Logger.hpp> to any files that need it")
        print("2. Initialize the logger in main() functions")
        print("3. Test the build and functionality")
        print("4. Review the log output for consistency")
    
    return 0

if __name__ == '__main__':
    sys.exit(main())

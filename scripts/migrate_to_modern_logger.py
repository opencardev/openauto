#!/usr/bin/env python3
"""
OpenAuto Modern Logger Migration Script

This script migrates all OPENAUTO_LOG calls to the new modern logger.
It performs the following transformations:
1. Replaces #include <f1x/openauto/Common/Log.hpp> with #include <openauto/Common/ModernLogger.hpp>
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
    'trace': 'OPENAUTO_LOG_TRACE',
    'debug': 'OPENAUTO_LOG_DEBUG', 
    'info': 'OPENAUTO_LOG_INFO',
    'warning': 'OPENAUTO_LOG_WARN',
    'error': 'OPENAUTO_LOG_ERROR',
    'fatal': 'OPENAUTO_LOG_FATAL'
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
    'Factory': 'SYSTEM',
    'Input': 'INPUT',
    'Projection': 'PROJECTION',
    'Settings': 'SETTINGS'
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
    # Handle complex stream operations
    content = stream_content.strip()
    
    # If it's a simple quoted string, extract it
    if content.startswith('"') and content.endswith('"') and content.count('"') == 2:
        return content[1:-1]
    
    # For complex expressions, preserve them as-is for the new macro
    return content

def convert_log_call(match, file_path):
    """Convert an OPENAUTO_LOG call to the new logger format"""
    severity = match.group(1).strip()
    stream_content = match.group(2).strip()
    
    # Map severity to new macro
    if severity not in SEVERITY_MAPPING:
        print(f"Warning: Unknown severity '{severity}' in {file_path}")
        return match.group(0)  # Return original if unknown
    
    new_macro = SEVERITY_MAPPING[severity]
    category = determine_category(file_path, stream_content)
    
    # For the new macro, we need to convert the stream to a string
    if stream_content.startswith('"') and stream_content.endswith('"'):
        # Simple string literal
        message = stream_content
    else:
        # Complex expression, wrap in parentheses and convert to string
        message = f'({stream_content}).str()'
        # If it looks like it's already building a string, use it directly
        if '<<' in stream_content:
            # This is a stream operation, we need to build the string
            message = f'(std::stringstream() << {stream_content}).str()'
    
    return f'{new_macro}({category}, {message})'

def process_file(file_path, dry_run=False):
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
            '#include <openauto/Common/ModernLogger.hpp>'
        )
        modified = True
        if dry_run:
            print(f"  - Would replace include statement")
    
    # Convert OPENAUTO_LOG calls
    # Pattern matches: OPENAUTO_LOG(severity) << content;
    pattern = r'OPENAUTO_LOG\s*\(\s*(\w+)\s*\)\s*<<([^;]+);'
    
    matches = list(re.finditer(pattern, content))
    if matches and dry_run:
        print(f"  - Would convert {len(matches)} OPENAUTO_LOG calls")
    
    def replace_func(match):
        nonlocal modified
        modified = True
        return convert_log_call(match, file_path)
    
    content = re.sub(pattern, replace_func, content)
    
    # Write back if modified and not dry run
    if modified and not dry_run:
        try:
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(content)
            print(f"Migrated: {file_path}")
            return True
        except Exception as e:
            print(f"Error writing {file_path}: {e}")
            return False
    
    return modified

def find_cpp_files(root_dir):
    """Find all C++ source files in the directory tree"""
    cpp_files = []
    for root, dirs, files in os.walk(root_dir):
        # Skip certain directories
        if any(skip_dir in root for skip_dir in ['build', '.git', 'cmake', 'tests']):
            continue
        
        for file in files:
            if file.endswith(('.cpp', '.c', '.cc', '.cxx', '.hpp', '.h')):
                cpp_files.append(os.path.join(root, file))
    
    return cpp_files

def main():
    parser = argparse.ArgumentParser(description='Migrate OpenAuto logging to modern logger')
    parser.add_argument('directory', nargs='?', default='.', help='Root directory to process (default: current directory)')
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
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                content = f.read()
            
            if ('OPENAUTO_LOG' in content or 
                '#include <f1x/openauto/Common/Log.hpp>' in content):
                
                if args.dry_run:
                    print(f"Would migrate: {file_path}")
                    if args.verbose:
                        process_file(file_path, dry_run=True)
                    migrated_count += 1
                else:
                    if process_file(file_path, dry_run=False):
                        migrated_count += 1
        except Exception as e:
            if args.verbose:
                print(f"Error processing {file_path}: {e}")
    
    print(f"\n{'Would migrate' if args.dry_run else 'Migrated'} {migrated_count} files")
    
    if not args.dry_run and migrated_count > 0:
        print("\nMigration complete! Next steps:")
        print("1. Update CMakeLists.txt to include the new ModernLogger source files")
        print("2. Initialize the logger in main() functions")
        print("3. Test the build and functionality")
        print("4. Review the log output for consistency")
        print("5. Consider configuring log levels per category")
    
    return 0

if __name__ == '__main__':
    sys.exit(main())
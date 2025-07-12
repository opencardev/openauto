#!/usr/bin/env python3
"""
Script to fix malformed logger statements with double quotes
"""
import re
import os
import glob

def fix_logger_quotes(filepath):
    """Fix malformed logger statements in a file"""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
        
        original_content = content
        
        # Fix patterns like: LOG_*(*, ""[text]
        content = re.sub(
            r'(LOG_[A-Z]+\([^,]+,\s*)""([^"]+)"',
            r'\1"\2"',
            content
        )
        
        # Fix patterns like: LOG_*(*, ""[text] << var << "text"");
        content = re.sub(
            r'(LOG_[A-Z]+\([^,]+,\s*)""([^"]+)([^)]+)"\);',
            r'\1"\2\3);',
            content
        )
        
        # Fix lines that end with ");"; and replace with );
        content = re.sub(
            r'"\);";',
            r'");',
            content
        )
        
        # Fix multi-line logger statements that start with ""
        content = re.sub(
            r'(LOG_[A-Z]+\([^,]+,\s*)""([^"]+<<[^)]*)\);"',
            r'\1"\2);',
            content,
            flags=re.MULTILINE | re.DOTALL
        )
        
        if content != original_content:
            with open(filepath, 'w', encoding='utf-8') as f:
                f.write(content)
            print(f"Fixed logger quotes in {filepath}")
            return True
        else:
            return False
    except Exception as e:
        print(f"Error processing {filepath}: {e}")
        return False

def main():
    """Main function"""
    # Find all C++ files in src directory
    cpp_files = []
    for root, dirs, files in os.walk('src'):
        for file in files:
            if file.endswith(('.cpp', '.hpp')):
                cpp_files.append(os.path.join(root, file))
    
    fixed_count = 0
    for filepath in cpp_files:
        if fix_logger_quotes(filepath):
            fixed_count += 1
    
    print(f"Fixed {fixed_count} files")

if __name__ == "__main__":
    main()

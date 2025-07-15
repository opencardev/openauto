#!/bin/bash

# Post-create script for OpenAuto development container
# This script runs after the container is created

set -e

echo "🚀 Running post-create setup for OpenAuto development environment..."

# Ensure we're in the workspace directory
cd /workspace

# Set up Git configuration (these can be overridden by user)
git config --global --add safe.directory /workspace
git config --global init.defaultBranch main
git config --global pull.rebase false
git config --global core.autocrlf input

echo "📦 Setting up project dependencies..."

# Create build directory if it doesn't exist
mkdir -p build

# Create config directory with development configuration
mkdir -p config
if [ ! -f config/development.json ]; then
    cat > config/development.json << 'EOF'
{
  "logger": {
    "level": "DEBUG",
    "file": "/workspace/logs/debug.log",
    "console": true,
    "async": false
  },
  "api": {
    "host": "0.0.0.0",
    "port": 8080,
    "cors": true,
    "debug": true
  },
  "android_auto": {
    "video_fps": 30,
    "video_resolution": "1280x720",
    "audio_sample_rate": 48000,
    "debug_mode": true
  },
  "event_bus": {
    "max_subscribers": 100,
    "queue_size": 1000,
    "debug": true
  },
  "configuration": {
    "auto_save": true,
    "backup_count": 5,
    "file": "/workspace/config/openauto.conf"
  }
}
EOF
fi

# Create logs directory
mkdir -p logs

# Initialize CMake configuration
echo "🔧 Configuring CMake build..."
cd build

# Configure with debug settings for development
cmake .. \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DBUILD_TESTS=ON \
    -DENABLE_COVERAGE=ON \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
    -DCMAKE_C_COMPILER_LAUNCHER=ccache \
    -GNinja

cd ..

# Set up pre-commit hooks
echo "🔍 Setting up pre-commit hooks..."
if command -v pre-commit >/dev/null 2>&1; then
    # Create .pre-commit-config.yaml if it doesn't exist
    if [ ! -f .pre-commit-config.yaml ]; then
        cat > .pre-commit-config.yaml << 'EOF'
repos:
  - repo: https://github.com/pre-commit/pre-commit-hooks
    rev: v4.4.0
    hooks:
      - id: trailing-whitespace
      - id: end-of-file-fixer
      - id: check-yaml
      - id: check-added-large-files
      - id: check-merge-conflict
  
  - repo: https://github.com/psf/black
    rev: 23.3.0
    hooks:
      - id: black
        language_version: python3
        files: \.py$
  
  - repo: https://github.com/pycqa/isort
    rev: 5.12.0
    hooks:
      - id: isort
        args: ["--profile", "black"]
        files: \.py$
  
  - repo: local
    hooks:
      - id: clang-format
        name: clang-format
        description: Format C++ code with clang-format
        entry: clang-format
        language: system
        files: \.(cpp|hpp|h|c)$
        args: ["-i"]
EOF
    fi
    
    # Install pre-commit hooks
    pre-commit install
fi

# Create VS Code workspace settings
echo "⚙️  Setting up VS Code configuration..."
mkdir -p .vscode

# Create tasks.json for common development tasks
if [ ! -f .vscode/tasks.json ]; then
    cat > .vscode/tasks.json << 'EOF'
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "Build Debug",
            "type": "shell",
            "command": "cmake",
            "args": ["--build", "build", "--parallel"],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "presentation": {
                "echo": true,
                "reveal": "always",
                "focus": false,
                "panel": "shared"
            },
            "problemMatcher": "$gcc"
        },
        {
            "label": "Build Release",
            "type": "shell",
            "command": "cmake",
            "args": ["--build", "build", "--config", "Release", "--parallel"],
            "group": "build",
            "presentation": {
                "echo": true,
                "reveal": "always",
                "focus": false,
                "panel": "shared"
            },
            "problemMatcher": "$gcc"
        },
        {
            "label": "Run Tests",
            "type": "shell",
            "command": "ctest",
            "args": ["--test-dir", "build", "--output-on-failure", "--parallel"],
            "group": "test",
            "presentation": {
                "echo": true,
                "reveal": "always",
                "focus": false,
                "panel": "shared"
            },
            "dependsOn": "Build Debug"
        },
        {
            "label": "Clean Build",
            "type": "shell",
            "command": "rm",
            "args": ["-rf", "build/*"],
            "group": "build",
            "presentation": {
                "echo": true,
                "reveal": "always",
                "focus": false,
                "panel": "shared"
            }
        },
        {
            "label": "Configure CMake",
            "type": "shell",
            "command": "cmake",
            "args": [
                "..",
                "-DCMAKE_BUILD_TYPE=Debug",
                "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
                "-DBUILD_TESTS=ON",
                "-DENABLE_COVERAGE=ON",
                "-GNinja"
            ],
            "options": {
                "cwd": "${workspaceFolder}/build"
            },
            "group": "build",
            "presentation": {
                "echo": true,
                "reveal": "always",
                "focus": false,
                "panel": "shared"
            }
        },
        {
            "label": "Run OpenAuto",
            "type": "shell",
            "command": "./build/bin/autoapp",
            "args": ["--config", "config/development.json"],
            "group": "test",
            "presentation": {
                "echo": true,
                "reveal": "always",
                "focus": true,
                "panel": "dedicated"
            },
            "dependsOn": "Build Debug"
        }
    ]
}
EOF
fi

# Create launch.json for debugging
if [ ! -f .vscode/launch.json ]; then
    cat > .vscode/launch.json << 'EOF'
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug OpenAuto",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/bin/autoapp",
            "args": ["--config", "${workspaceFolder}/config/development.json"],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [
                {
                    "name": "OPENAUTO_LOG_LEVEL",
                    "value": "DEBUG"
                }
            ],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                },
                {
                    "description": "Set Disassembly Flavor to Intel",
                    "text": "set disassembly-flavor intel",
                    "ignoreFailures": true
                }
            ],
            "preLaunchTask": "Build Debug",
            "miDebuggerPath": "/usr/bin/gdb"
        },
        {
            "name": "Debug Unit Tests",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/tests/unit/unit_tests",
            "args": ["--gtest_output=xml:${workspaceFolder}/test_results.xml"],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Enable pretty-printing for gdb",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ],
            "preLaunchTask": "Build Debug",
            "miDebuggerPath": "/usr/bin/gdb"
        }
    ]
}
EOF
fi

# Set up useful aliases
echo "🔗 Setting up useful aliases..."
cat >> ~/.zshrc << 'EOF'

# OpenAuto development aliases
alias build='cmake --build build --parallel'
alias test='ctest --test-dir build --output-on-failure'
alias clean='rm -rf build/* && cmake -B build -S . -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -GNinja'
alias run='./build/bin/autoapp --config config/development.json'
alias debug='gdb ./build/bin/autoapp'
alias format='find src include tests -name "*.cpp" -o -name "*.hpp" -o -name "*.h" | xargs clang-format -i'
alias analyze='cppcheck --enable=all --std=c++17 src/'

# Useful development shortcuts
alias ll='ls -alF'
alias la='ls -A'
alias l='ls -CF'
alias ..='cd ..'
alias ...='cd ../..'
alias grep='grep --color=auto'
alias fgrep='fgrep --color=auto'
alias egrep='egrep --color=auto'

# Git shortcuts
alias gs='git status'
alias ga='git add'
alias gc='git commit'
alias gp='git push'
alias gl='git log --oneline --graph'
alias gd='git diff'

EOF

# Create helpful scripts
echo "📜 Creating helper scripts..."
mkdir -p scripts

cat > scripts/build.sh << 'EOF'
#!/bin/bash
set -e

BUILD_TYPE=${1:-Debug}
echo "Building OpenAuto in $BUILD_TYPE mode..."

cd build
cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DBUILD_TESTS=ON -GNinja
cmake --build . --parallel $(nproc)

echo "Build complete! Binaries are in build/bin/"
EOF

cat > scripts/test.sh << 'EOF'
#!/bin/bash
set -e

echo "Running OpenAuto tests..."

cd build
ctest --output-on-failure --parallel $(nproc)

echo "All tests completed!"
EOF

cat > scripts/format.sh << 'EOF'
#!/bin/bash
set -e

echo "Formatting code with clang-format..."

find src include tests -name "*.cpp" -o -name "*.hpp" -o -name "*.h" | xargs clang-format -i

echo "Code formatting complete!"
EOF

chmod +x scripts/*.sh

# Create README for the development environment
cat > README.dev.md << 'EOF'
# OpenAuto Development Environment

This development environment provides everything needed to build and develop OpenAuto.

## Quick Start

1. **Build the project**: `F1` -> `Tasks: Run Task` -> `Build Debug`
2. **Run tests**: `F1` -> `Tasks: Run Task` -> `Run Tests`
3. **Debug**: `F5` to start debugging with the default configuration

## Available Commands

- `build` - Build the project
- `test` - Run all tests
- `clean` - Clean build directory and reconfigure
- `run` - Run OpenAuto with development config
- `format` - Format all source code

## Directory Structure

- `src/` - Source code
- `include/` - Header files
- `tests/` - Test files
- `build/` - Build artifacts
- `config/` - Configuration files
- `logs/` - Log files
- `scripts/` - Helper scripts

## Development Workflow

1. Make changes to source code
2. Build with `Ctrl+Shift+P` -> `Tasks: Run Build Task`
3. Run tests with `Ctrl+Shift+P` -> `Tasks: Run Task` -> `Run Tests`
4. Debug if needed with `F5`

## Container Features

- Multi-architecture support (amd64, arm64)
- All dependencies pre-installed
- ccache for faster rebuilds
- Pre-commit hooks for code quality
- VS Code integration
- GDB debugging support

## Port Forwarding

- 8080: OpenAuto REST API
- 5555: Android Auto communication
- 3000: Development server

Happy coding! 🚗
EOF

echo "✅ Post-create setup completed successfully!"
echo ""
echo "🎉 Welcome to the OpenAuto development environment!"
echo ""
echo "Next steps:"
echo "  1. Build the project: Run task 'Build Debug'"
echo "  2. Run tests: Run task 'Run Tests'"
echo "  3. Start debugging: Press F5"
echo ""
echo "For help, see README.dev.md"

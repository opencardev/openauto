# Install clang-format on Windows

## Option 1: Using Visual Studio Installer
1. Open Visual Studio Installer
2. Modify your Visual Studio installation
3. Under "Individual components", search for "Clang"
4. Install "C++ Clang tools for VS 2019/2022"

## Option 2: Using Chocolatey
```powershell
# Install chocolatey first if not already installed
Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))

# Install LLVM (includes clang-format)
choco install llvm
```

## Option 3: Using winget
```powershell
winget install LLVM.LLVM
```

## Option 4: Manual Download
1. Go to https://github.com/llvm/llvm-project/releases
2. Download the Windows pre-built binaries
3. Extract and add to PATH

After installation, restart your terminal and run:
```powershell
clang-format --version
```

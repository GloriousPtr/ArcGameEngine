@echo off
ECHO Downloading and installing LLVM
if not exist ".cache" mkdir .cache
curl -fsSL -o ".cache/LLVM-16.0.0-win64.exe" "https://github.com/llvm/llvm-project/releases/download/llvmorg-16.0.0/LLVM-16.0.0-win64.exe"
".cache/LLVM-16.0.0-win64.exe" /S
pause

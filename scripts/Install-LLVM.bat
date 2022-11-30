@echo off
ECHO Downloading and installing LLVM
if not exist ".cache" mkdir .cache
curl -fsSL -o ".cache/LLVM-15.0.5-win64.exe" "https://github.com/llvm/llvm-project/releases/download/llvmorg-15.0.5/LLVM-15.0.5-win64.exe"
".cache/LLVM-15.0.5-win64.exe" /S
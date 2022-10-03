@echo off
pushd %~dp0\..\

set "compiler=%1%"
if "%compiler%" == "" set /p compiler= Enter Compiler (msvc/clang): 

echo:
echo ================================================================
echo Generating Solution and project files for %compiler%
echo ================================================================
echo:


set "compilerCommand=none"
if "%compiler%" == "clang" set "compilerCommand=--cc=clang"
if "%compiler%" == "msvc" set "compilerCommand="

echo ================================================================
if "%compilerCommand%" == "none" (echo Unsupported compiler %compiler%) else (call vendor\premake\bin\premake5.exe %compilerCommand% vs2022)
echo ================================================================
echo:

popd
PAUSE

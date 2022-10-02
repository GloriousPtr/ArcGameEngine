@echo off
pushd %~dp0\..\

set "compiler=%1%"
if "%compiler%" == "" set /p compiler= Enter Compiler: 
if "%compiler%" == "clang" set "compiler=--cc=clang"
if "%compiler%" == "msvc" set "compiler="

call vendor\premake\bin\premake5.exe %compiler% vs2022

popd
PAUSE

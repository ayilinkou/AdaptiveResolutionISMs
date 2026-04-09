@echo off

REM Set command
REM Put your generator of choice after -G
set CMAKE_CMD=cmake -G "Visual Studio 17 2022" -A x64 -B ./build .

REM Run cmake generate command
echo %CMAKE_CMD%
%CMAKE_CMD%

REM only pause if this is the top-level batch
if "%~0" == "%~f0" pause
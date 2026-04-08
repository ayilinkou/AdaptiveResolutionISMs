@echo off

REM Set command
REM Put your generator of choice after -G
set CMAKE_CMD=cmake -G "Visual Studio 17 2022" -A x64 -B ./build .

REM Bootstrap vcpkg if not done already
set VCPKG_ROOT=%~dp0vcpkg

if not exist "%VCPKG_ROOT%\vcpkg.exe" (
    echo vcpkg not found, bootstrapping...
    call "%VCPKG_ROOT%\bootstrap-vcpkg.bat"
) else (
    echo vcpkg already bootstrapped.
)

REM Run cmake generate command
echo %CMAKE_CMD%
%CMAKE_CMD%

pause

@echo off

REM Delete old build folder for a clean build
rmdir /s /q build

REM Generate solution
call GENERATE_SLN.bat

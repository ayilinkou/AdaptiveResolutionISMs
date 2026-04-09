@echo off

REM Delete old build folder for a clean build
if exist .\build rmdir /s /q build

REM Generate solution
call GENERATE_SLN.bat

REM only pause if this is the top-level batch
if "%~0" == "%~f0" pause

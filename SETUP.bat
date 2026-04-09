@echo off

REM Bootstrap vcpkg if not done already
set VCPKG_ROOT=%~dp0vcpkg

REM Checks if submodules are pulled, tries to pull them if don't exist
if not exist "%VCPKG_ROOT%\vcpkg.exe" (
    echo vcpkg.exe not found, bootstrapping...
    if not exist "%VCPKG_ROOT%\bootstrap-vcpkg.bat" (
	echo bootstrap-vcpkg.bat not found! Perhaps cloned repo without --recurse-submodules?
	echo Pulling submodules...	
	git submodule update --init --recursive
	if not exist "%VCPKG_ROOT%\bootstrap-vcpkg.bat" (
	    echo Failed to pull submodules! Try cloning this repo again with the --recurse-submodules option.
	    pause
	    exit
	)
    )
    call "%VCPKG_ROOT%\bootstrap-vcpkg.bat" -disableMetrics
) else (
    echo vcpkg already bootstrapped.
)

REM Generate solution
call GENERATE_SLN_CLEAN.bat

REM Download models
call DOWNLOAD_MODELS.bat

REM only pause if this is the top-level batch
if "%~0" == "%~f0" pause
@echo off
setlocal

mkdir temp

set EMERALD_URL=https://developer.nvidia.com/emerald-square
set EMERALD_ZIP=.\temp\emerald-square.zip
set BISTRO_URL=https://developer.nvidia.com/bistro
set BISTRO_ZIP=.\temp\bistro.zip
set MODELS_FOLDER=App\Models

echo Downloading models...

curl --parallel	-L -o %BISTRO_ZIP% %BISTRO_URL% ^
     		-L -o %EMERALD_ZIP% %EMERALD_URL%

echo Download finished!
echo Unzipping models...

if not exist %MODELS_FOLDER% mkdir %MODELS_FOLDER%
tar -xf %BISTRO_ZIP% -C %MODELS_FOLDER%
tar -xf %EMERALD_ZIP% -C %MODELS_FOLDER%

echo Unzipping complete!

rmdir /s /q temp

REM only pause if this is the top-level batch
if "%~0" == "%~f0" pause
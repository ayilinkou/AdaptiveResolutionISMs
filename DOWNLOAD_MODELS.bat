@echo off
setlocal

mkdir temp

set EMERALD_URL=https://developer.nvidia.com/emerald-square
set EMERALD_ZIP=.\temp\emerald-square.zip
set BISTRO_URL=https://developer.nvidia.com/bistro
set BISTRO_ZIP=.\temp\bistro.zip
set SAN_MIGUEL_URL=https://casual-effects.com/g3d/data10/research/model/San_Miguel/San_Miguel.zip
set SAN_MIGUEL_ZIP=.\temp\San_Miguel.zip
set MODELS_FOLDER=App\Models
set SAN_MIGUEL_FOLDER=%MODELS_FOLDER%\San_Miguel

echo Downloading models...

curl --parallel	-L -o %BISTRO_ZIP% %BISTRO_URL% ^
     		-L -o %EMERALD_ZIP% %EMERALD_URL% ^
		-L -o %SAN_MIGUEL_ZIP% %SAN_MIGUEL_URL%

echo Download finished!
echo Unzipping models...

if not exist %MODELS_FOLDER% mkdir %MODELS_FOLDER%
if not exist %SAN_MIGUEL_FOLDER% mkdir %SAN_MIGUEL_FOLDER%
tar -xf %BISTRO_ZIP% -C %MODELS_FOLDER%
tar -xf %EMERALD_ZIP% -C %MODELS_FOLDER%
tar -xf %SAN_MIGUEL_ZIP% -C %SAN_MIGUEL_FOLDER%

echo Unzipping complete!

rmdir /s /q temp

REM only pause if this is the top-level batch
if "%~0" == "%~f0" pause
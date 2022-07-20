:: Based on GeoReferencing plugin vcpkg.bat
:: https://github.com/EpicGames/UnrealEngine/blob/release/Engine/Plugins/Runtime/GeoReferencing/Source/ThirdParty/vcpkg.bat
@echo off

:: this is where the artifacts get installed
set VCPKG_INSTALLED=vcpkg-installed

set MERGETIFF_VERSION=v0.0.6

:: Build mode used by vcpkg
set BUILD_CONFIG=x64-windows-static-md

:: cleanup the git repo
if exist "%~dp0vcpkg\" echo:
if exist "%~dp0vcpkg\" echo === Tidying up vcpkg ===
if exist "%~dp0vcpkg\" rmdir /s /q "%~dp0vcpkg"

:: cleanup the prior artifacts
if exist "%~dp0%VCPKG_INSTALLED%\" echo:
if exist "%~dp0%VCPKG_INSTALLED%\" echo === Tidying up %VCPKG_INSTALLED% ===
if exist "%~dp0%VCPKG_INSTALLED%\" rmdir /s /q "%~dp0%VCPKG_INSTALLED%"

echo:
echo === Cloning vcpkg to %~dp0vcpkg ===
git clone https://github.com/microsoft/vcpkg.git %~dp0vcpkg"

echo:
echo === Bootstrapping vcpkg ===
:: -disableMetrics in important to avoid Malwarebytes quarantine the vcpkg file. 
call "%~dp0vcpkg\bootstrap-vcpkg.bat" -disableMetrics

echo:
echo === Running vcpkg ===
"%~dp0vcpkg\vcpkg.exe" install gdal[core]:%BUILD_CONFIG% --x-install-root="%~dp0%VCPKG_INSTALLED%" 
if ERRORLEVEL 1 exit /b 1

:: Clone Mergetiff
echo:
echo === Cloning mergetiff to %~dp0mergetiff ===
git clone https://github.com/adamrehn/mergetiff-cxx.git --branch %MERGETIFF_VERSION% "%~dp0mergetiff"

echo:
echo === Adding mergetiff to include folder ===
ren "%~dp0mergetiff\source\lib" mergetiff
move "%~dp0mergetiff\source\mergetiff" "%~dp0%VCPKG_INSTALLED%\%BUILD_CONFIG%\include" 

echo:
echo === Cleaning up files ===

:: Delete libcurl and zlib .lib files
del "%~dp0%VCPKG_INSTALLED%\%BUILD_CONFIG%\lib\libcurl.lib"
del "%~dp0%VCPKG_INSTALLED%\%BUILD_CONFIG%\lib\zlib.lib"

:: Delete libcurl include files
rmdir /s /q "%~dp0%VCPKG_INSTALLED%\%BUILD_CONFIG%\include\curl"

:: Delete zlib include files
del "%~dp0%VCPKG_INSTALLED%\%BUILD_CONFIG%\include\zlib.h"
del "%~dp0%VCPKG_INSTALLED%\%BUILD_CONFIG%\include\zconf.h"

:: Delete build/debug files to save space
rmdir /s /q "%~dp0%VCPKG_INSTALLED%\x64-windows"
rmdir /s /q "%~dp0%VCPKG_INSTALLED%\vcpkg"
rmdir /s /q "%~dp0%VCPKG_INSTALLED%\%BUILD_CONFIG%\debug"
rmdir /s /q "%~dp0%VCPKG_INSTALLED%\%BUILD_CONFIG%\tools"
rmdir /s /q "%~dp0mergetiff"
rmdir /s /q "%~dp0vcpkg"

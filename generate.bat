@echo off
setlocal
pushd "%~dp0"

git submodule sync --recursive
if errorlevel 1 goto :error

git submodule update --init --recursive
if errorlevel 1 goto :error

where premake5 >null 2>null
if %ERRORLEVEL%==1 (
  echo Premake5 was not found. You must install premake5 to compile boiii.
  goto :error
)

premake5 %* vs2022
if errorlevel 1 goto :error

popd
exit /b 0

:error
set "exit_code=%errorlevel%"
popd
echo Project generation failed.
exit /b %exit_code%

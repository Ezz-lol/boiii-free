@echo off
setlocal
pushd "%~dp0"

echo Generating Ninja build files...
premake5 %* ninja 
set "exit_code=%errorlevel%"
if not "%exit_code%"=="0" goto :finish

ninja Release_x64 -Cbuild
set "exit_code=%errorlevel%"

:finish
echo.
if "%exit_code%"=="0" (
  echo Build completed successfully.
  echo Output: build\bin\x64\Release\boiii.exe
) else (
  echo Build failed with exit code %exit_code%.
)
echo.
popd
pause
exit /b %exit_code%

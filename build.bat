@echo off
setlocal
pushd "%~dp0"

echo Generating Visual Studio project files...
call generate.bat
set "exit_code=%errorlevel%"
if not "%exit_code%"=="0" goto :finish

echo.
echo Building boiii - Release x64...
powershell.exe -NoProfile -ExecutionPolicy Bypass -Command ^
  "$ErrorActionPreference = 'Stop';" ^
  "$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe';" ^
  "if (-not (Test-Path -LiteralPath $vswhere)) { throw 'Visual Studio or Build Tools was not found. Install the Desktop development with C++ workload.' };" ^
  "$msbuild = & $vswhere -latest -products * -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1;" ^
  "if (-not $msbuild) { throw 'MSBuild was not found. Install the Desktop development with C++ workload.' };" ^
  "& $msbuild 'build\boiii.sln' /m /v:normal /p:Configuration=Release /p:Platform=x64;" ^
  "exit $LASTEXITCODE"
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

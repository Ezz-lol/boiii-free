@echo off
setlocal
pushd "%~dp0"

git submodule sync --recursive
if errorlevel 1 goto :error

git submodule update --init --recursive
if errorlevel 1 goto :error

if not exist "tools\premake5.exe" (
  echo Premake 5 was not found. Downloading the pinned version...
  powershell.exe -NoProfile -ExecutionPolicy Bypass -Command ^
    "$ErrorActionPreference = 'Stop';" ^
    "$url = 'https://github.com/premake/premake-core/releases/download/v5.0.0-beta8/premake-5.0.0-beta8-windows.zip';" ^
    "$expected = 'e64ce2ed8778e0098f63674cca61fe33941b5f0c8d9a4afd651152bdea3758ab';" ^
    "$temp = Join-Path ([IO.Path]::GetTempPath()) ('boiii-premake-' + [guid]::NewGuid());" ^
    "try { New-Item -ItemType Directory -Force -Path $temp, 'tools' | Out-Null; $zip = Join-Path $temp 'premake.zip'; Invoke-WebRequest -UseBasicParsing -Uri $url -OutFile $zip; $sha = [Security.Cryptography.SHA256]::Create(); $stream = [IO.File]::OpenRead($zip); try { $actual = ([BitConverter]::ToString($sha.ComputeHash($stream))).Replace('-', '').ToLowerInvariant() } finally { $stream.Dispose(); $sha.Dispose() }; if ($actual -ne $expected) { throw 'Premake download checksum mismatch.' }; Expand-Archive -LiteralPath $zip -DestinationPath $temp -Force; Copy-Item -LiteralPath (Join-Path $temp 'premake5.exe') -Destination 'tools\premake5.exe' } finally { if (Test-Path -LiteralPath $temp) { Remove-Item -LiteralPath $temp -Recurse -Force } }"
  if errorlevel 1 goto :error
)

"tools\premake5.exe" %* vs2022
if errorlevel 1 goto :error

popd
exit /b 0

:error
set "exit_code=%errorlevel%"
popd
echo Project generation failed.
exit /b %exit_code%

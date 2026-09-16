$ErrorActionPreference = "Stop"

# Source the environment helpers
."$PSScriptRoot\env.ps1"

$RepoDir = Get-RepoDir

# Check all required dependencies (excluding 'kill', which translates to built-in process management in PS)
Check-Dependencies -Deps @(
    "git",
    "clang-format",
    "stylua",
    "shfmt",
    "yarn",
    "dos2unix"
)

$MaxParallelJobs = Get-NumThreads

# Install yarn dependencies
Push-Location $RepoDir
try {
    # Using cmd /c guarantees proper exit code resolution for .cmd wrappers on Windows
    cmd / c "yarn install"
    if ($LASTEXITCODE -ne 0) {
        throw "yarn install failed with exit code $LASTEXITCODE"
    }
} catch {
    Write-Error "Error: Failed to install `yarn` dependencies. Exiting."
    Pop-Location
    exit 1
}
Pop-Location

# Filter function equivalent to should_format

function Should-Format {
    param([string] $Path)

    if (-not [string]::IsNullOrWhiteSpace($Path)) {
        # Normalize slashes for regex matching
        $pathStr = $Path -replace '\\', '/'

        if (
            -not ($pathStr -match "^deps/") -or (
                $pathStr -match "^deps/premake/"
            )
        ) {
            return $true
        }
    }
    return $false
}

# Fetch all tracked files
$files = git -C $RepoDir ls-files --deduplicate --exclude-standard

$filesToFormat = @()
foreach ($file in $files) {
    if (Should-Format -Path $file) {
        $absPath = [System.IO.Path]::GetFullPath(
            [System.IO.Path]::Combine( $RepoDir, $file )
        )
        if (Test-Path -Path $absPath -PathType Leaf) {
            $filesToFormat += $absPath
        }
    }
}

# Define the scriptblock that will execute concurrently in runspaces
$formatScriptBlock = {
    param(
        $path,
        $repoDir
    )

    $ext = [System.IO.Path]::GetExtension($path).TrimStart('.').ToLower()

    # Helper function to enforce exit code checks within the runspace

    function Invoke-CommandOrThrow {
        param([scriptblock] $cmd)

        & $cmd
        if ($LASTEXITCODE -ne 0) {
            throw "Command failed with exit code $LASTEXITCODE"
        }
    }

    Push-Location $repoDir
    try {
        switch -Regex ($ext) {
            "^lua$" {
                Invoke-CommandOrThrow {
                    stylua $path
                }
                Invoke-CommandOrThrow {
                    dos2unix --quiet $path
                }
                break
            }
            "^(md|markdown|html|htm|js|jsx|ts|tsx|mjs|cjs|mts|cts|graphql|gql|json|json5|webmanifest|yaml|yml|vue|svelte|handlebars|hbs|toml|ps1)$" {
                $prettierConfig = Join-Path $repoDir ".prettierrc.json"
                # Call yarn via cmd to properly resolve the batch wrapper in runspaces
                Invoke-CommandOrThrow {
                    cmd / c "yarn run prettier --config `"$prettierConfig`" --write `"$path`""
                }
                Invoke-CommandOrThrow {
                    dos2unix --quiet $path
                }
                break
            }
            "^sh$" {
                Invoke-CommandOrThrow {
                    shfmt --write $path
                }
                break
            }
            "^(c|cc|cpp|cxx|hpp|h|hxx|js|json|proto|java|cs|objc|sv|svh|v|svp|svl|vlg|vt|vh|verilog|vo|vqm|vcd)$" {
                Invoke-CommandOrThrow {
                    clang-format -i --style = file $path
                }
                Invoke-CommandOrThrow {
                    dos2unix --quiet $path
                }
                break
            }
        }
    } catch {
        Write-Error "Failed to format $path : $_"
    } finally {
        Pop-Location
    }
}

# Initialize RunspacePool for highly-efficient parallel execution
$pool = [runspacefactory]::CreateRunspacePool( 1, $MaxParallelJobs )
$pool.Open()
$jobs = @()

try {
    # Queue up all formatting jobs
    foreach ($file in $filesToFormat) {
        $ps = [powershell]::Create().AddScript($formatScriptBlock).AddArgument(
            $file
        ).AddArgument($RepoDir)
        $ps.RunspacePool = $pool
        $jobs += [PSCustomObject] @{
            Pipe = $ps
            AsyncResult = $ps.BeginInvoke()
        }
    }

    # Wait for all jobs to complete (equivalent to wait_all_jobs)
    foreach ($job in $jobs) {
        $job.Pipe.EndInvoke($job.AsyncResult)
        $job.Pipe.Dispose()
    }
} finally {
    # Cleanup jobs mapping to kill_all_jobs trap
    $pool.Close()
    $pool.Dispose()
}

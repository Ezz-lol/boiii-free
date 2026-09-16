function Get-ScriptDir {
    return $PSScriptRoot
}

function Get-RepoDir {
    $scriptDir = Get-ScriptDir
    $repoDir = git -C "$scriptDir" rev-parse --show-toplevel
    return $repoDir.Trim()
}

function Check-Dependencies {
    param(
        [string[]] $Deps = @( "git", "clang-format", "stylua" )
    )

    foreach ($tool in $Deps) {
        if (-not (Get-Command "$tool" -ErrorAction SilentlyContinue)) {
            Write-Error "Error: Required dependency '$tool' is not installed or not in PATH."
            exit 1
        }
    }
}

function Get-NumThreads {
    if ($env:NUMBER_OF_PROCESSORS) {
        return [int] $env:NUMBER_OF_PROCESSORS
    }
    return 1
}

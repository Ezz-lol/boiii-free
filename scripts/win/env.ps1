function Get-ScriptDir {
    return "$PSScriptRoot"
}

function Get-RepoDir {
    $scriptDir = Get-ScriptDir
    return git -C "$scriptDir" rev-parse --show-toplevel
}

function Check-Dependencies {
    $deps = @("git", "clang-format", "stylua")
    foreach ($tool in $deps) {
        if (-not (Get-Command "$tool" -ErrorAction SilentlyContinue)) {
            Write-Error "Error: Required dependency '$tool' is not installed or not in PATH."
            exit 1
        }
    }
}

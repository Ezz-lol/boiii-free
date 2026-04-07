$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$RepoRoot = git -C "$ScriptDir" rev-parse --show-toplevel
$EnvFile = Join-Path "$ScriptDir" "env.ps1"
# Load environment
. "$EnvFile"
Check-Dependencies

$RepoGitDir = Join-Path "$RepoRoot" ".git"
$GitHooksDir = Join-Path "$RepoRoot" ".git/hooks"
$ScriptsDir = Join-Path "$RepoRoot" "scripts/win"

#exit with error if $RepoRoot does not exist or is not a directory or git repository (.git folder does not exist)
if (-not (Test-Path "$RepoRoot" -PathType Container) -or -not (Test-Path "$RepoGitDir" -PathType Container)) {
    Write-Error "Error: Repository root directory not found or is not a git repository."
    exit 1
}

# Ensure target exists
if (-not (Test-Path "$GitHooksDir" -PathType Container)) {
    New-Item -ItemType Directory -Path $GitHooksDir -Force
}

$SourceHooksDir = Join-Path "$RepoRoot" "hooks"
$SourcePrecommitHook = Join-Path "$SourceHooksDir" "pre-commit"
$PreCommitHookOutputPath = Join-Path "$GitHooksDir" "pre-commit"


# Copy files
Copy-Item -Path "$SourcePrecommitHook" -Destination "$PreCommitHookOutputPath" -Force
Write-Host "Git hooks successfully installed." -ForegroundColor Green

#!/bin/bash

script_dir() {
  local script_dirname
  script_dirname="$(dirname "$0")"
  if [ -L "$script_dirname" ]; then
    script_dirname="$(readlink -f "$script_dirname")"
  fi

  realpath "$script_dirname"
}

# Returns the root of the containing git repository
repo_dir() {
  local repo_dir
  repo_dir="$(git -C "$(script_dir)" rev-parse --show-toplevel)"
  if [ -L "$repo_dir" ]; then
    repo_dir="$(readlink -f "$repo_dir")"
  fi
  realpath "$repo_dir"
}

. "$(repo_dir)/scripts/unix/env.sh"

REPO_ROOT="$(repo_dir)"
# Ensure git and clang-format are present
check_dependencies

# Ensure the hooks directory exists in .git
mkdir -p "${REPO_ROOT}/.git/hooks"

# Copy the pre-commit hook
cp "${REPO_ROOT}/hooks/pre-commit" "${REPO_ROOT}/.git/hooks/pre-commit"

# Make the hook executable
chmod +x "${REPO_ROOT}/.git/hooks/pre-commit"

echo "Git hooks successfully installed."

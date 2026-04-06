#!/bin/bash

# Returns the absolute path of the directory containing this script
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
  local script_dir_path
  local earliest_possible_worktree_dir

  script_dir_path="$(script_dir)"
  earliest_possible_worktree_dir="$(remove_git_dir_from_path "$script_dir_path")"
  repo_dir="$(git -C "${earliest_possible_worktree_dir}" rev-parse --show-toplevel)"
  if [ -L "$repo_dir" ]; then
    repo_dir="$(readlink -f "$repo_dir")"
  fi
  repo_dir="$(realpath "$repo_dir")"
  if ! [ -e "$repo_dir" ] || ! [ -d "$repo_dir" ] || ! [ -d "${repo_dir}/.git" ]; then
    echo "Error: Unable to determine the root of the git repository."
    exit 1
  fi

  echo "$repo_dir"
}

safe_grep() {
  ARGS=()
  while [ "$#" -gt 0 ]; do
    ARGS+=("$1")
    shift
  done

  grep -E "${ARGS[@]}" 2>/dev/null || true
}

# Verifies required CLI tools are installed
check_dependencies() {
  local deps=("git" "clang-format")
  for tool in "${deps[@]}"; do
    if ! command -v "$tool" &>/dev/null; then
      echo "Error: Required dependency '$tool' is not installed or not in PATH."
      exit 1
    fi
  done
}

remove_git_dir_from_path() {
  local PATH_PARTS
  PATH_PARTS=()
  IFS='/' read -ra PATH_PARTS <<<"$1"
  local CLEANED_PATH=""
  for PART in "${PATH_PARTS[@]}"; do
    if [ "$PART" == ".git" ]; then
      break
    fi
    CLEANED_PATH="${CLEANED_PATH}/${PART}"
  done
  realpath "$CLEANED_PATH"
}

#!/bin/bash

first_in_dir() {
	local search_dir
	search_dir="$(normalize_path "$1")"
	shift
	local search_files
	search_files=()
	while [ "$#" -gt 0 ]; do
		search_files+=("$1")
		shift
	done

	if [ -z "$search_dir" ]; then
		echo "Error: No search directory provided to first_in_dir." >&2
		exit 1
	fi

	if [ "${#search_files[@]}" -eq 0 ]; then
		echo "Error: No search files provided to first_in_dir." >&2
		exit 1
	fi

	for file in "${search_files[@]}"; do
		if [ -e "$search_dir/$file" ]; then
			normalize_path "$search_dir/$file"
			return 0
		fi
	done

	return 1
}

normalize_path() {
	local file_path
	file_path="$1"

	if [ -z "$file_path" ]; then
		file_path="$(cat -)"
	fi

	if [ -z "$file_path" ]; then
		echo "Error: No file path provided to normalize_path." >&2
		exit 1
	fi

	if [ -L "$file_path" ]; then
		file_path="$(readlink -f "$file_path")"
	fi

	# Fix Git Bash path translation bug where C:/ gets converted to //c/
	if [[ "$file_path" == //* ]]; then
		file_path="/${file_path:2}"
	fi

	realpath "$file_path"
}

resolve_path() {
	local path_to_resolve
	path_to_resolve="$1"

	if [ -z "$path_to_resolve" ]; then
		path_to_resolve="$(cat -)"
	fi

	if [ -z "$path_to_resolve" ]; then
		echo "Error: No path provided to resolve_path." >&2
		return 1
	fi

	resolved="$(which "$path_to_resolve" 2>/dev/null || command -v "$path_to_resolve" 2>/dev/null)"
	if [ -z "$resolved" ]; then
		echo "Error: Could not resolve path for '$path_to_resolve'." >&2
		return 1
	fi

	normalize_path "$resolved"
	return 0
}

ext() {
	local file_path
	file_path="$1"

	if [ -z "$file_path" ]; then
		file_path="$(cat -)"
	fi

	if [ -z "$file_path" ]; then
		echo "Error: No file path provided to ext." >&2
		exit 1
	fi

	echo "${file_path##*.}"
}

starts_with() {
	local str
	str="$1"
	local prefix
	prefix="$2"

	if [ -z "$str" ]; then
		str="$(cat -)"
	fi

	if [ -z "$str" ]; then
		echo "Error: No string provided to starts_with." >&2
		exit 1
	fi

	if [ -z "$prefix" ]; then
		echo "Error: No prefix provided to starts_with." >&2
		exit 1
	fi

	[[ "$str" == "$prefix"* ]]
}

to_lowercase() {
	local input_str
	input_str="$1"

	if [ -z "$input_str" ]; then
		input_str="$(cat -)"
	fi

	if [ -z "$input_str" ]; then
		echo "Error: No input string provided to to_lowercase." >&2
		exit 1
	fi

	echo "${input_str,,}"
}

to_uppercase() {
	local input_str
	input_str="$1"

	if [ -z "$input_str" ]; then
		input_str="$(cat -)"
	fi

	if [ -z "$input_str" ]; then
		echo "Error: No input string provided to to_uppercase." >&2
		exit 1
	fi

	echo "${input_str^^}"
}

# Returns the absolute path of the directory containing this script
script_dir() {
	normalize_path "$(dirname "$0")"
}

path_in_tree_by_name() {
	local input_path
	input_path="$(normalize_path "$1")"

	local dir_name
	dir_name="$2"

	if [ -z "$input_path" ]; then
		echo "Error: No input path provided to path_in_tree_by_name." >&2
		exit 1
	fi

	if [ -z "$dir_name" ]; then
		echo "Error: No directory name provided to path_in_tree_by_name." >&2
		exit 1
	fi

	local PATH_PARTS
	PATH_PARTS=()
	IFS='/' read -ra PATH_PARTS <<<"$input_path"

	for part in "${PATH_PARTS[@]}"; do
		if [[ "$part" == "$dir_name" ]]; then
			return 0
		fi
	done

	return 1
}

path_remove_tree_by_name() {
	local input_path
	input_path="$(normalize_path "$1")"

	local dir_name
	dir_name="$2"

	if [ -z "$input_path" ]; then
		echo "Error: No input path provided to path_remove_tree_by_name." >&2
		exit 1
	fi

	if [ -z "$dir_name" ]; then
		echo "Error: No directory name provided to path_remove_tree_by_name." >&2
		exit 1
	fi

	local PATH_PARTS
	PATH_PARTS=()
	IFS='/' read -ra PATH_PARTS <<<"$input_path"

	local out_path
	out_path=""

	for part in "${PATH_PARTS[@]}"; do
		if [[ "$part" == "$dir_name" ]]; then
			break
		fi
		out_path="${out_path}/${part}"
	done
	normalize_path "$out_path"
}

# Verifies required CLI tools are installed
check_dependencies() {
	local deps=("git" "clang-format" "stylua")
	while [ "$#" -gt 0 ]; do
		deps+=("$1")
		shift
	done

	for tool in "${deps[@]}"; do
		if ! command -v "$tool" &>/dev/null; then
			echo "Error: Required dependency '$tool' is not installed or not in PATH." >&2
			exit 1
		fi
	done
}

num_threads() {
	getconf NPROCESSORS_ONLN 2>/dev/null ||
		sysctl -n hw.ncpu 2>/dev/null ||
		nproc 2>/dev/null ||
		getconf _NPROCESSORS_ONLN 2>/dev/null ||
		echo 1
}

chdir() {
	local path="$1"
	shift

	args=()
	while [ "$#" -gt 0 ]; do
		args+=("$1")
		shift
	done

	local init_pwd="$(pwd)"

	if ! cd "$path"; then
		return 1
	fi

	local return_code=0

	if ! "${args[@]}"; then
		return_code=1
	fi

	cd "$init_pwd" || true

	return "$return_code"
}

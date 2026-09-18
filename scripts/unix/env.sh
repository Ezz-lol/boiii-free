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
		return 1
	fi

	if [ "${#search_files[@]}" -eq 0 ]; then
		echo "Error: No search files provided to first_in_dir." >&2
		return 1
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
	file_path=""

	if [ "$#" -gt 0 ]; then
		file_path="$1"
	else
		file_path="$(cat -)"
	fi

	if [ -z "$file_path" ]; then
		echo "Error: No file path provided to normalize_path." >&2
		return 1
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
	path_to_resolve=""

	if [ "$#" -gt 0 ]; then
		path_to_resolve="$1"
	else
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
	file_path=""

	if [ "$#" -gt 0 ]; then
		file_path="$1"
	else
		file_path="$(cat -)"
	fi

	if [ -z "$file_path" ]; then
		echo "Error: No file path provided to ext." >&2
		return 1
	fi

	echo "${file_path##*.}"
}

starts_with() {
	local str
	str=""

	if [ "$#" -gt 0 ]; then
		str="$1"
	else
		str="$(cat -)"
	fi

	local prefix
	prefix="$2"

	if [ -z "$str" ]; then
		echo "Error: No string provided to starts_with." >&2
		return 1
	fi

	if [ -z "$prefix" ]; then
		echo "Error: No prefix provided to starts_with." >&2
		return 1
	fi

	[[ "$str" == "$prefix"* ]]
}

to_lowercase() {
	local input_str
	input_str=""

	if [ "$#" -gt 0 ]; then
		input_str="$1"
	else
		input_str="$(cat -)"
	fi

	if [ -z "$input_str" ]; then
		echo "Error: No input string provided to to_lowercase." >&2
		return 1
	fi

	echo "${input_str,,}"
}

to_uppercase() {
	local input_str
	input_str=""

	if [ "$#" -gt 0 ]; then
		input_str="$1"
	else
		input_str="$(cat -)"
	fi

	if [ -z "$input_str" ]; then
		echo "Error: No input string provided to to_uppercase." >&2
		return 1
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
		return 1
	fi

	if [ -z "$dir_name" ]; then
		echo "Error: No directory name provided to path_in_tree_by_name." >&2
		return 1
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
		return 1
	fi

	if [ -z "$dir_name" ]; then
		echo "Error: No directory name provided to path_remove_tree_by_name." >&2
		return 1
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

# MacOS `realpath` does not support the `--relative-to` argument.
# We use this instead to ensure we are not reliant on this GNU extension.
relative_path() {
	local relative_to="$1"
	local path="$2"

	# Guard against empty paths preventing pipeline hangs
	if [ -z "$relative_to" ] || [ -z "$path" ]; then
		echo "."
		return 1
	fi

	# Safely resolve absolute paths (avoids realpath errors on missing files)
	if [ -e "$relative_to" ] || [ -L "$relative_to" ]; then
		relative_to="$(normalize_path "$relative_to")"
	elif [[ "$relative_to" != /* ]]; then
		relative_to="$(normalize_path "$PWD")/${relative_to#./}"
	fi

	if [ -e "$path" ] || [ -L "$path" ]; then
		path="$(normalize_path "$path")"
	elif [[ "$path" != /* ]]; then
		path="$(normalize_path "$PWD")/${path#./}"
	fi

	local common_prefix="$path"

	# Find the longest common leading subdirectory prefix safely
	while [[ "${relative_to}/" != "${common_prefix%/}/"* ]]; do
		common_prefix="${common_prefix%/*}"
		[ -z "$common_prefix" ] && common_prefix="/"
	done

	# Get the non-common remaining segments of both paths, stripping leading slashes
	local path_remainder="${path#"$common_prefix"}"
	path_remainder="${path_remainder#/}"
	local relative_to_remainder="${relative_to#"$common_prefix"}"
	relative_to_remainder="${relative_to_remainder#/}"

	local result=""

	# For directory directory remaining in relative_to, climb up one directory ('../')
	if [ -n "$relative_to_remainder" ]; then
		local p="$relative_to_remainder"
		while [[ "$p" == */* ]]; do
			result="../$result"
			p="${p#*/}"
		done
		result="../$result"
	fi

	result="${result}${path_remainder}"

	# Strip trailing slash(es) if result non-empty
	result="${result%/}"
	echo "${result:-.}"
}

# MacOS `ln` does not support the `--relative` argument.
# We use this instead to ensure we are not reliant on this GNU extension.
lnrs() {
	if [ "$#" -ne 2 ]; then
		echo "Usage: lnrs <target> <link_path>" >&2
		return 1
	fi

	local target="$1"
	local link_path="$2"
	local link_dir
	local link_name

	# If link_path is a directory, link becomes file with basename of target
	# within the directory. Matches behaviour of `ln`.
	if [ -d "$link_path" ]; then
		link_dir="$link_path"
		link_name="$(basename "$target")"
	else
		link_dir="$(dirname "$link_path")"
		link_name="$(basename "$link_path")"
	fi

	local rel_path="$(relative_path "$link_dir" "$target")"

	chdir "$link_dir" \
		ln -s "$rel_path" "$link_name"
}

have_application() {
	command -v "$1" >/dev/null 2>&1
}

is_gnu() {
	local application="$1"

	if [ -n "$application" ] && have_application "$application"; then
		local version_output="$("$application" --version 2>&1 || true)"
		version_output="$(to_lowercase "$version_output")"
		if [[ "$version_output" == *"gnu"* ]] || [[ "$version_output" == *"free software foundation"* ]]; then
			return 0
		fi
	fi

	return 1
}

host_triple() {
	if have_application "clang"; then
		clang -dumpmachine
	elif have_application "gcc"; then
		gcc -dumpmachine
	elif have_application "rustc"; then
		rustc -vV | sed -n 's/^host: //p'
	elif have_application uname; then
		echo "$(uname -m)-unknown-$(uname -s | to_lowercase)"
	fi
}

tree_depth() {
	local target_dir="$(normalize_path "${1:-.}")"

	# Ensure the target is a valid directory
	if [ ! -d "$target_dir" ]; then
		echo "Error: Directory '$target_dir' not found." >&2
		return 1
	fi

	# Calculate max depth
	local max_depth
	max_depth=$(chdir "$target_dir" find . -type d | awk -F"/" '{print NF-1}' | sort -nr | head -n 1)

	echo "$((max_depth + 1))"
}

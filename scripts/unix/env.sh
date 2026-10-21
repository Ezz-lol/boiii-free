#!/bin/bash

fmt_version() {
	local result
	result=""

	if [ "$#" -gt 0 ]; then
		result="$1"
		if [ "$#" -gt 1 ]; then
			result="${result}.${2}"
			if [ "$#" -gt 2 ]; then
				result="${result}.${3}"
			fi
		fi
	fi

	echo "$result"

}

bash_version_major() {
	echo "${BASH_VERSINFO[0]}"
}

bash_version_minor() {
	echo "${BASH_VERSINFO[1]}"
}

ensure_minimum_bash_version() {
	local major
	local minor

	major="$1"
	minor="$2"

	local curr_major
	curr_major="$(bash_version_major)"
	local curr_minor
	curr_minor="$(bash_version_minor)"

	local failed=0
	if [ "$curr_major" -lt "$major" ]; then
		failed=1
	elif [ "$curr_major" -eq "$major" ] && [ "$curr_minor" -lt "$minor" ]; then
		failed=1
	fi

	if [ "$failed" -eq 1 ]; then
		echo "Error: This script requires Bash version $(fmt_version "$major" "$minor") or higher." >&2
		echo "Your current version is: $(fmt_version "$curr_major" "$curr_minor")" >&2
		exit 1
	fi
}

# Portable replacement for `readlink -f`.
#
# Canonicalizes the given path by recursively following every symlink in
# every component. Matches GNU `readlink -f` semantics:
#
#   * relative input is relative to the value of the $PWD environment variable
#   * `.` and `..` are collapsed; `..` never escapes `/`
#   * redundant and trailing slashes are removed
#   * every component except the last must exist - the last may be missing
#   * symlink loops are detected (limited to 40)
#
# Usage: readlink_recursive <path>
# Prints the canonical absolute path on stdout; returns 1 on error.
readlink_recursive() {
	local path="${1:-}"
	local max_symlinks=40
	local symlink_count=0

	if [ -z "$path" ]; then
		echo "Error: No path provided to readlink_recursive." >&2
		return 1
	fi

	# Anchor to an absolute path
	if [[ "$path" != /* ]]; then
		path="${PWD}/${path}"
	fi

	local -a stack=()
	local remaining="$path"
	local component
	local cur_path
	local link_target
	local top
	local seg

	while [ -n "$remaining" ]; do
		# Strip leading slashes (handles `//foo`, `///foo`, etc.).
		# The nested expansion takes the longest prefix of non-slash
		# characters (the first component) and strips everything before it.
		remaining="${remaining#"${remaining%%[!/]*}"}"

		if [ -z "$remaining" ]; then
			break
		fi

		# Extract the first component
		if [[ "$remaining" == */* ]]; then
			component="${remaining%%/*}"
			remaining="${remaining#*/}"
		else
			component="$remaining"
			remaining=""
		fi

		case "$component" in
		'' | .)
			# Empty or current-dir component: skip
			continue
			;;
		..)
			# Parent-dir component: pop one resolved segment.
			# Popping an empty stack stays at `/`.
			if [ "${#stack[@]}" -gt 0 ]; then
				top=$((${#stack[@]} - 1))
				unset "stack[$top]"
			fi
			continue
			;;
		esac

		# Build the current absolute path from the resolved stack plus
		# the component under consideration.
		cur_path=""
		if [ "${#stack[@]}" -gt 0 ]; then
			for seg in "${stack[@]}"; do
				cur_path+="/$seg"
			done
		fi
		cur_path+="/$component"

		# Is it a symlink?
		if [ -L "$cur_path" ]; then
			symlink_count=$((symlink_count + 1))
			if [ "$symlink_count" -gt "$max_symlinks" ]; then
				echo "Error: Too many symlinks while resolving '$1'." >&2
				return 1
			fi

			if ! link_target="$(readlink "$cur_path")"; then
				echo "Error: Failed to read symlink '$cur_path'." >&2
				return 1
			fi

			if [[ "$link_target" == /* ]]; then
				# Absolute target: reset the resolved stack.
				stack=()
				if [ -n "$remaining" ]; then
					remaining="${link_target}/${remaining}"
				else
					remaining="$link_target"
				fi
			else
				# Relative target: keep the current parent (already on
				# the stack) and prepend the relative target.
				if [ -n "$remaining" ]; then
					remaining="${link_target}/${remaining}"
				else
					remaining="$link_target"
				fi
			fi
			continue
		fi

		# Non-symlink component. If there is more to process, this
		# component is not the last, so it must exist.
		if [ -n "$remaining" ] && ! [ -e "$cur_path" ]; then
			echo "Error: Path component '$cur_path' does not exist." >&2
			return 1
		fi

		stack+=("$component")
	done

	if [ "${#stack[@]}" -eq 0 ]; then
		echo "/"
	else
		printf '/%s' "${stack[@]}"
		printf '\n'
	fi
}

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

	for search_filename in "${search_files[@]}"; do
		search_dir_file="${search_dir}/${search_filename}"
		if [ -e "$search_dir_file" ]; then
			normalize_path "$search_dir_file"
			return 0
		fi
	done

	return 1
}

have_application() {
	command -v "$1" >/dev/null 2>&1
}

is_gnu() {
	local application="$1"

	if [ -n "$application" ] && have_application "$application"; then
		local version_output
		version_output="$("$application" --version 2>&1 || true)"
		version_output="$(to_lowercase "$version_output")"
		if [[ "$version_output" == *"gnu"* ]] || [[ "$version_output" == *"free software foundation"* ]]; then
			return 0
		fi
	fi

	return 1
}

is_llvm() {
	local application="$1"

	if [ -n "$application" ] && have_application "$application"; then
		local version_output
		version_output="$("$application" --version 2>&1 || true)"
		if [ -n "$version_output" ]; then
			if [[ "$version_output" == *"LLVM version"* ]] || [[ "$version_output" == *"clang version"* ]]; then
				return 0
			fi
		fi
	fi

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
		file_path="$(readlink_recursive "$file_path")"
	fi

	# Fix Git Bash path translation bug where C:/ gets converted to //c/
	if [[ "$file_path" == //* ]]; then
		file_path="/${file_path:2}"
	fi

	# Select best available means of successful canonicalization of non-existent path
	if (have_application "realpath" && is_gnu "realpath") || [[ "$(realpath --help 2>&1)" == *'canonicalize-missing'* ]]; then
		realpath --canonicalize-missing "$file_path"
	elif have_application "grealpath" && is_gnu "grealpath"; then
		grealpath --canonicalize-missing "$file_path"
	else
		readlink_recursive "$file_path"
	fi
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

	local resolved
	resolved="$(command -v "$path_to_resolve" 2>/dev/null || true)"
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
	local prefix

	case "$#" in
	2)
		str="$1"
		prefix="$2"
		;;
	1)
		prefix="$1"
		str="$(cat -)"
		;;
	*)
		echo "Usage: starts_with <prefix>  (or: printf '%s' <str> | starts_with <prefix>)" >&2
		return 1
		;;
	esac

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

# shellcheck disable=SC2120
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
	out_path="/"

	for part in "${PATH_PARTS[@]}"; do
		if [ -n "$part" ]; then
			if [[ "$part" == "$dir_name" ]]; then
				break
			fi
			out_path="${out_path%/}/${part}"
		fi
	done
	normalize_path "$out_path"
}

# Verifies required CLI tools are installed
MIN_BASH_MAJOR_VERSION=4
MIN_BASH_MINOR_VERSION=0
check_dependencies() {
	# Bash 4.0 minimum, as we require declarative arrays and string case conversion expansions ("${s,,}" or "{s^^}")
	ensure_minimum_bash_version "$MIN_BASH_MAJOR_VERSION" "$MIN_BASH_MINOR_VERSION"
	local -a deps=("git")
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

	local -a args
	args=()
	while [ "$#" -gt 0 ]; do
		args+=("$1")
		shift
	done

	local init_pwd
	init_pwd="$(pwd)" || return 1

	if ! cd "$path"; then
		return 1
	fi

	local return_code=0

	if [ "${#args[@]}" -gt 0 ]; then
		"${args[@]}" || return_code=$?
	fi

	cd "$init_pwd" >/dev/null 2>&1 || true

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

	local rel_path
	rel_path="$(relative_path "$link_dir" "$target")"

	chdir "$link_dir" \
		ln -s "$rel_path" "$link_name"
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
	local target_dir
	local depth

	target_dir="$(normalize_path "${1:-.}")" || return 1

	if [ ! -d "$target_dir" ]; then
		echo "Error: Directory '$target_dir' not found." >&2
		return 1
	fi

	depth="$(
		chdir "$target_dir" find . -type d -print |
			awk -F/ '{ d=NF-1; if (d>max) max=d } END { print max+0 }'
	)" || return 1

	echo "$((depth + 1))"
}

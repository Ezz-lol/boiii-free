#!/usr/bin/env bash

set -euo pipefail

script_dir() {
	local script_dirname
	script_dirname="$(dirname "$0")"

	if ! realpath "$script_dirname" 2>/dev/null; then
		echo "$script_dirname"
	fi
}

# Returns the root of the containing git repository
repo_dir() {
	local repo_dir
	repo_dir="$(git -C "$(script_dir)" rev-parse --show-toplevel 2>/dev/null)"
	if [ -z "$repo_dir" ] || ! realpath "$repo_dir" 2>/dev/null; then
		echo "$repo_dir"
	fi
}

sha256_file() {
	if have_application sha256sum; then
		sha256sum "$1" | awk '{print $1}'
	elif have_application shasum; then
		shasum -a 256 "$1" | awk '{print $1}'
	elif have_application openssl; then
		openssl dgst -sha256 "$1" | awk '{print $NF}'
	else
		with_temp_coreutils sha256sum "$1" | awk '{print $1}'
	fi
}

PRUNE=0
prune() {
	[ "$PRUNE" -eq 1 ]
}

print_usage() {
	echo "x86_64-unknown-windows-msvc toolchain system root generator."
	echo "Usage: $0 [OPTIONS]"
	echo "Options:"
	echo "  --output,-output,-o        Specify output sysroot path."
	echo "  --prune,-prune,-p          Prune output libraries, restricting to only those actively in use by boiii."
	echo "  --help, -h                 Show this help message and exit."
}

TEMP_COREUTILS_PATH=""
TEMP_COREUTILS_BIN=""
XWIN_CACHE=""

RUST_COREUTILS_ORG="uutils"
RUST_COREUTILS_REPO="coreutils"
RUST_COREUTILS_API_URI="https://api.github.com/repos/${RUST_COREUTILS_ORG}/${RUST_COREUTILS_REPO}/releases"
COREUTILS_DOWNLOAD_DEPS=("jq" "curl" "tar" "cut" "sed" "unzip" "find")

download_temp_coreutils() {
	local search_host_triple
	local latest_assets
	local asset_count
	local platform_asset_name
	local platform_asset_uri
	local asset_idx
	local asset_name
	local asset_uri
	local asset_triple
	local temp_asset_out
	local asset_ext

	echo "GNU coreutils missing. Attempting to download a temporary GNU-compatible coreutils binary for your platform." 1>&2

	check_dependencies "${COREUTILS_DOWNLOAD_DEPS[@]}"

	search_host_triple="$(
		sed \
			-e 's/^arm64-/aarch64-/' \
			-e 's/-pc-/-unknown-/g' \
			-e 's/-gnu$/-musl/' \
			-e 's/-darwin[0-9][0-9.]*$/-darwin/' \
			<<<"$HOST_TRIPLE"
	)"

	latest_assets="$(curl -sSL -H "Accept: application/vnd.github+json" "$RUST_COREUTILS_API_URI" | jq -r '.[0].assets')"
	asset_count="$(jq -r 'length' <<<"$latest_assets")"

	platform_asset_name=""
	platform_asset_uri=""
	for ((asset_idx = 0; asset_idx < asset_count; asset_idx++)); do
		asset_name="$(jq -r ".[$asset_idx].name" <<<"$latest_assets")"
		asset_uri="$(jq -r ".[$asset_idx].browser_download_url" <<<"$latest_assets")"

		asset_triple="${asset_name#coreutils-}"
		asset_triple="$(
			sed \
				-e 's/^[0-9][0-9.]*-//' \
				-e 's/\.tar\.[^.]*$//' \
				-e 's/\.zip$//' \
				-e 's/^arm64-/aarch64-/' \
				<<<"$asset_triple"
		)"
		if [[ "$asset_triple" == "$search_host_triple" ]]; then
			platform_asset_name="$asset_name"
			platform_asset_uri="$asset_uri"
			break
		fi
	done

	if [ -n "$platform_asset_name" ] && [ -n "$platform_asset_uri" ]; then
		temp_asset_out="/tmp/${platform_asset_name}"
		if ! curl -sSL "$platform_asset_uri" -o "$temp_asset_out"; then
			echo "Error: GNU coreutils missing, and failed to download for your current platform. Install GNU coreutils, then try again." >&2
			rm "$temp_asset_out" 2>/dev/null || true
			return 1
		fi

		mkdir -p "$TEMP_COREUTILS_PATH"

		asset_ext="$(ext "$platform_asset_name" | to_lowercase)"
		if [[ "$asset_ext" == "zip" ]]; then
			if ! unzip "$temp_asset_out" -d "$TEMP_COREUTILS_PATH"; then
				echo "Error: GNU coreutils missing, and failed to extract download for your current platform. Install GNU coreutils, then try again." >&2
				rm "$temp_asset_out" 2>/dev/null || true
				return 1
			fi
		elif [[ "$platform_asset_name" == *".tar."* ]]; then
			if ! tar -xv -f "$temp_asset_out" -C "$TEMP_COREUTILS_PATH"; then
				echo "Error: GNU coreutils missing, and failed to extract download for your current platform. Install GNU coreutils, then try again." >&2
				rm "$temp_asset_out" 2>/dev/null || true
				return 1
			fi
		fi

		rm "$temp_asset_out" 2>/dev/null || true
		TEMP_COREUTILS_BIN="$(find "$TEMP_COREUTILS_PATH" -type f \( -iname "coreutils" -o -iname "coreutils.exe" \) -print -quit)"
		echo "Successfully downloaded and extracted temporary GNU-compatible coreutils binary for your platform." >&2

	else
		echo "Error: GNU coreutils missing, and could not find a download for your current platform. Install GNU coreutils, then try again." >&2
		return 1
	fi
}

with_temp_coreutils() {
	if [ -z "$TEMP_COREUTILS_BIN" ] && [ -d "$TEMP_COREUTILS_PATH" ]; then
		TEMP_COREUTILS_BIN="$(find "$TEMP_COREUTILS_PATH" -type f \( -iname "coreutils" -o -iname "coreutils.exe" \) -print -quit)"
	fi

	if [ -z "$TEMP_COREUTILS_BIN" ] && ! download_temp_coreutils; then
		return 1
	fi

	local -a args
	args=()

	while [ "$#" -gt 0 ]; do
		args+=("$1")
		shift
	done

	"${TEMP_COREUTILS_BIN}" "${args[@]}"
}

cleanup_exit() {
	local exit_code
	exit_code=0

	if [ "$#" -gt 0 ]; then
		exit_code="$1"
	fi

	local -a cleanup_dirs
	cleanup_dirs=("$XWIN_CACHE" "$TEMP_COREUTILS_PATH")

	for cleanup_dir in "${cleanup_dirs[@]}"; do
		if [ -n "$cleanup_dir" ] && [ -d "$cleanup_dir" ]; then
			rm -rf "$cleanup_dir" >/dev/null 2>&1 || true
		fi
	done

	exit "$exit_code"
}

# MacOS `sort` supports a smaller subset of options supported by GNU `sort`,
# so we try to find and use `gsort` instead if possible, and otherwise fallback to
# a temporary coreutils binary.
gnu_sort() {
	local -a args
	args=()

	while [ "$#" -gt 0 ]; do
		args+=("$1")
		shift
	done

	if have_application "gsort" && is_gnu "gsort"; then
		gsort "${args[@]}"
	elif have_application "sort" && is_gnu "sort"; then
		sort "${args[@]}"
	else
		with_temp_coreutils "sort" "${args[@]}"
	fi
}

dedup_dir() {
	local target_dir
	target_dir="$1"

	local hash
	local original

	if [ -n "$target_dir" ]; then
		target_dir="$(normalize_path "$target_dir")"
		if [ -d "$target_dir" ]; then

			declare -A file_hashes=()

			# Find only regular files to prevent touching existing symlinks or directories
			while IFS='' read -r -d '' cmp_file; do
				if [ -n "$cmp_file" ] && [ -f "$cmp_file" ]; then
					hash="$(sha256_file "$cmp_file")"

					if [ -n "${file_hashes[$hash]+set}" ]; then
						original="${file_hashes[$hash]}"
						echo "Linking duplicate: $original -> $cmp_file"

						rm "$cmp_file"

						lnrs "$original" "$cmp_file"
					else
						file_hashes[$hash]="$cmp_file"
					fi
				fi
			done < <(find "$target_dir" -type f -print0 | gnu_sort -zhbf)
		fi
	fi
}

declare -A LIBRARIES_IN_USE=(["advapi32"]=1
	["asmjit"]=1
	["atls"]=1
	["bcrypt"]=1
	["comctl32"]=1
	["common"]=1
	["crypt32"]=1
	["d3d11"]=1
	["d3dcompiler"]=1
	["dxgi"]=1
	["dbghelp"]=1
	["dwmapi"]=1
	["gdi32"]=1
	["imm32"]=1
	["iphlpapi"]=1
	["kernel32"]=1
	["libcmt"]=1
	["libcmtd"]=1
	["libcpmt"]=1
	["libcpmtd"]=1
	["libucrt"]=1
	["libucrtd"]=1
	["libvcruntime"]=1
	["libvcruntimed"]=1
	["ntdll"]=1
	["oldnames"]=1
	["ole32"]=1
	["oleaut32"]=1
	["psapi"]=1
	["shell32"]=1
	["shlwapi"]=1
	["urlmon"]=1
	["user32"]=1
	["uuid"]=1
	["version"]=1
	["winhttp"]=1
	["winmm"]=1
	["ws2_32"]=1)

declare -A NECESSARY_SYSROOT_FILE_EXTENSIONS=(
	["h"]=1
	["rh"]=1
	["hpp"]=1
	["lib"]=1
	["pdb"]=1
)

UNNECESSARY_HEADERS=(
	"winrt"
	"CodeAnalysis"
	"experimental"
	"msclr"
	"gl"
	"winsqlite"
	"netcx"
)

UNNECESSARY_LIBRARIES=(
	"LIBCMT.lib"
	"LIBCMTD.lib"
	"UUID.lib"
	"enclave"
)

necessary_sysroot_file() {
	local file_name
	local file_ext
	local file_stem
	file_name="$1"

	file_ext="$(basename "$file_name" | ext | to_lowercase)"

	if [ -n "$file_ext" ]; then
		if [ -n "${NECESSARY_SYSROOT_FILE_EXTENSIONS["$file_ext"]+set}" ] && [ "${NECESSARY_SYSROOT_FILE_EXTENSIONS["$file_ext"]}" -eq 1 ]; then
			case "$file_ext" in
			lib | pdb)
				file_stem="$(stem "$file_name" | to_lowercase)"
				if [ -n "${LIBRARIES_IN_USE["$file_stem"]+set}" ] && [ "${LIBRARIES_IN_USE["$file_stem"]}" -eq 1 ]; then
					return 0
				fi
				;;
			*)
				return 0
				;;
			esac
		fi
		return 1
	fi
	return 0
}

remove_broken_symlinks() {
	local root
	root="$1"

	while IFS='' read -r -d '' entry; do
		echo "Pruning broken symlink: $entry"
		rm "$entry" >/dev/null 2>&1 || true
	done < <(find "$root" -type l ! -exec test -e {} \; -print)
}

remove_empty_directories() {
	local root
	root="$1"

	while IFS='' read -r -d '' entry; do
		echo "Pruning empty directory: $entry"
		rm -rf "$entry" >/dev/null 2>&1 || true
	done < <(find "$root" -type d -empty -print0)
}

remove_unnecessary_headers() {
	local sysroot
	sysroot="$1"

	local unnecessary_header_abs=""
	for unnecessary_header in "${UNNECESSARY_HEADERS[@]}"; do
		if [ -n "$unnecessary_header" ]; then
			unnecessary_header_abs="${sysroot}/include/${unnecessary_header}"
			if [ -e "$unnecessary_header_abs" ]; then
				echo "Pruning unnecessary header: ${unnecessary_header_abs}"
				rm -rf "$unnecessary_header_abs" >/dev/null 2>&1 || true
			fi
		fi
	done

	while IFS='' read -r -d '' entry; do
		echo "Pruning unnecessary header: $entry"
		rm "$entry" >/dev/null 2>&1 || true
	done < <(find "${sysroot}/include" \
		-type f \
		\( \
		-iname "Windows.UI.*" \
		-o -iname "*.xaml*." \
		\) \
		-print0)
}

remove_unnecessary_libraries() {
	local sysroot
	sysroot="$1"

	local unnecessary_library_abs=""
	for unnecessary_library in "${UNNECESSARY_LIBRARIES[@]}"; do
		if [ -n "$unnecessary_library" ]; then
			unnecessary_library_abs="${sysroot}/lib/${unnecessary_library}"
			if [ -e "$unnecessary_library_abs" ]; then
				echo "Pruning unnecessary library: ${unnecessary_library_abs}"
				rm -rf "$unnecessary_library_abs" >/dev/null 2>&1 || true
			fi
		fi
	done

}

prune_sysroot() {
	local sysroot
	sysroot="$1"

	remove_unnecessary_headers "$sysroot"
	remove_unnecessary_libraries "$sysroot"

	# Remove files which are not necessary libraries or headers
	while IFS='' read -r -d '' entry; do
		if [ -n "$entry" ] && [ -f "$entry" ] && ! necessary_sysroot_file "$entry"; then
			echo "Pruning unnecessary file: $entry"
			rm -rf "$entry" >/dev/null 2>&1 || true
		fi
	done < <(find "$sysroot" -type f -print0)

	remove_broken_symlinks "$sysroot"
	remove_empty_directories "$sysroot"
}

remove_intrinsic_headers() {
	if [ -d "$OUTPUT_SYSROOT_INCLUDE_PATH" ]; then
		while IFS='' read -r -d '' intrin; do
			if [ -n "$intrin" ]; then
				if [ -f "$intrin" ] || [ -L "$intrin" ]; then
					rm "$intrin"
				fi
			fi
		done < <(find "$OUTPUT_SYSROOT_INCLUDE_PATH" \
			\( -iname "*intrin.h" -o -iname "intrin*.h" \) \
			-print0)
	fi
}

link_lowercase() {
	local root
	local lower_tree_file

	root="$1"

	while IFS='' read -r -d '' tree_file; do
		if [ -n "$tree_file" ]; then
			lower_tree_file="$(dirname "$tree_file")/$(basename "$tree_file" | to_lowercase)"

			if [[ "$lower_tree_file" != "$tree_file" ]] &&
				! [ -e "$lower_tree_file" ] &&
				[ -e "$tree_file" ]; then
				echo "Linking lowercase: $tree_file -> $lower_tree_file"
				lnrs "$tree_file" "$lower_tree_file"
			fi
		fi
	done < <(find "$root" -mindepth 1 -print0)

}

link_case_sensitive() {
	local lower
	local upper

	lower="$(normalize_path "$1")"
	upper="$(normalize_path "$2")"

	if [ -e "$lower" ] && ! [ -e "$upper" ] && ! [ -L "$upper" ]; then
		echo "Linking case sensitive: $lower -> $upper"
		lnrs "$lower" "$upper"
	elif [ -e "$upper" ] && ! [ -e "$lower" ] && ! [ -L "$lower" ]; then
		echo "Linking case sensitive: $upper -> $lower"
		lnrs "$upper" "$lower"
	fi
}

link_special_case_sensitive() {
	local -a args
	local lower

	args=()
	while [ "$#" -gt 0 ]; do
		args+=("$1")
		shift
	done

	for cased in "${args[@]}"; do
		lower="$(dirname "$cased")/$(basename "$cased" | to_lowercase)"
		link_case_sensitive "$lower" "$cased"
	done
}

SYSROOT_DIRS=(
	"bin"
	"include"
	"lib"
	"share"
	"opt"
)

filter_sysroot_dirs() {
	local dir
	local dir_basename
	local sysroot_dir
	local depth
	local slashes

	declare -A wanted_names=()
	declare -A best_depth=()
	declare -A best_path=()

	for sysroot_dir in "${SYSROOT_DIRS[@]}"; do
		wanted_names[$sysroot_dir]=1
	done

	while IFS='' read -r -d '' dir; do
		if [ -n "$dir" ]; then
			dir_basename="$(basename "$dir")"
			if [ -n "${wanted_names[$dir_basename]+set}" ]; then
				slashes="${dir//[!/]/}"
				depth="${#slashes}"

				if [ -z "${best_depth[$dir_basename]+set}" ] ||
					[ "$depth" -lt "${best_depth[$dir_basename]}" ]; then
					best_depth[$dir_basename]="$depth"
					best_path[$dir_basename]="$dir"
				fi
			fi
		fi
	done

	for sysroot_dir in "${SYSROOT_DIRS[@]}"; do
		if [ -n "${best_path[$sysroot_dir]+set}" ]; then
			printf '%s\0' "${best_path[$sysroot_dir]}"
		fi
	done
}

SCRIPT_DIR="$(script_dir)"
REPO_DIR="$(repo_dir)"
if [ -n "$REPO_DIR" ]; then
	. "${REPO_DIR}/scripts/unix/env.sh"
else
	. "${SCRIPT_DIR}/env.sh"
fi

BUILD_DEPS=("rsync" "xwin" "sort")
check_dependencies "${BUILD_DEPS[@]}"

HOST_TRIPLE="$(host_triple)"

if [ -n "$REPO_DIR" ]; then
	XWIN_CACHE="${REPO_DIR}/build/xwin-cache"
else
	XWIN_CACHE="$(mktemp -d)"
fi

trap 'cleanup_exit 1' INT TERM HUP QUIT

mkdir -p "${XWIN_CACHE}"

OUTPUT_SYSROOT=""

while [ "$#" -gt 0 ]; do

	case "$1" in
	--output | -output | -o)
		if [ "$#" -gt 1 ]; then
			OUTPUT_SYSROOT="$2"
			shift 2
		else
			shift
		fi
		;;
	--prune | -prune | -p)
		PRUNE=1
		shift
		;;
	--help | -help | -h)
		print_usage
		cleanup_exit 0
		;;
	*)
		print_usage >&2
		cleanup_exit 1
		;;
	esac
done

if [ -z "$OUTPUT_SYSROOT" ]; then
	OUTPUT_SYSROOT="/opt/x86_64-unknown-windows-msvc"
fi

OUTPUT_SYSROOT="$(normalize_path "$OUTPUT_SYSROOT")"

OUTPUT_SYSROOT_INCLUDE_PATH="${OUTPUT_SYSROOT}/include"
OUTPUT_SYSROOT_LIB_PATH="${OUTPUT_SYSROOT}/lib"

SPECIAL_CASE_SENSITIVE_FILES=(
	"${OUTPUT_SYSROOT_INCLUDE_PATH}/Windows.h"
	"${OUTPUT_SYSROOT_INCLUDE_PATH}/OAIdl.h"
	"${OUTPUT_SYSROOT_INCLUDE_PATH}/OCIdl.h"
	"${OUTPUT_SYSROOT_LIB_PATH}/Crypt32.lib"
	"${OUTPUT_SYSROOT_LIB_PATH}/Crypt32.Lib"
)

if [ -n "$REPO_DIR" ]; then
	TEMP_COREUTILS_PATH="${REPO_DIR}/build/coreutils"
else
	TEMP_COREUTILS_PATH="$(mktemp -d)"
fi

mkdir -p "$OUTPUT_SYSROOT"

XWIN_SPLAT_DIR="${XWIN_CACHE}/splat"

XWIN_CMD=("xwin"
	"--cache-dir" "$XWIN_CACHE"
	"--accept-license"
	"--channel=release"
	"--arch=x86_64"
	"--include-atl"
	"--variant" "desktop"
	"--http-retry" "10"
	"--timeout" "1h")

if ! prune; then
	XWIN_CMD+=("--include-debug-runtime")
fi

XWIN_CMD+=("splat"
	"--include-debug-libs"
	"--include-debug-symbols")
if ! "${XWIN_CMD[@]}" || ! [ -d "$XWIN_SPLAT_DIR" ]; then
	echo "Error: xwin download, unpack, or splat failed. Exiting." 1>&2
	cleanup_exit 1
fi

while IFS='' read -r -d '' dist; do
	if [ -n "$dist" ] && [ -d "$dist" ]; then
		while IFS='' read -r -d '' sysroot_dir; do
			if [ -n "$sysroot_dir" ]; then
				sysroot_dirname="$(basename "$sysroot_dir")"
				out_sysroot_dir="${OUTPUT_SYSROOT}/${sysroot_dirname}"

				# Find highest directory depth that contains at least one file.
				# MSVC often distributes its packages with actual sysroot directory
				# contents nested many directories under the sysroot.
				#
				# These directories are intended to be flattened up to and including the
				# depth of the first directory, depth-wise, that contains files at its root.
				min_file_depth=1
				sysroot_dir_depth="$(tree_depth "$sysroot_dir")"
				while [ "$min_file_depth" -le "$sysroot_dir_depth" ] &&
					[ -z "$(find "$sysroot_dir" -mindepth "$min_file_depth" -maxdepth "$min_file_depth" -type f -print -quit)" ]; do
					min_file_depth=$((min_file_depth + 1))
				done
				min_file_dir_depth=$((min_file_depth - 1))

				if [ "$min_file_dir_depth" -lt "$sysroot_dir_depth" ]; then
					while IFS='' read -r -d '' wrapped_sysroot_dir; do
						if [ -n "$wrapped_sysroot_dir" ]; then
							rsync -avc --progress "${wrapped_sysroot_dir}/" "${out_sysroot_dir}/"
						fi
					done < <(find "$sysroot_dir" -mindepth "$min_file_dir_depth" -maxdepth "$min_file_dir_depth" -type d -print0)
				fi

			fi

			# Standard root-level sysroot directories - e.g. "include", "lib" - are
			# also often nested beneath either an architecture-specific directory (e.g. "x64"),
			# a directory named as the package version, or both.
			#
			# We use `filter_sysroot_dirs` to filter tree directories to those matching
			# standard root-level sysroot directory names, with at most one match per
			# name, where the match with lowest depth is selected.
		done < <(find "$dist" -mindepth 1 -type d -print0 | filter_sysroot_dirs)
	fi
done < <(find "$XWIN_SPLAT_DIR" -mindepth 1 -maxdepth 1 -type d -print0)

# These are provided by clang separately.
# Keeping these causes the compiler to use those provided by MSVC by default.
# This leads to definition conflicts, and thus compilation errors.
if ! remove_intrinsic_headers; then
	echo "Error: failed to remove intrinsic headers. Exiting." >&2
	cleanup_exit 1
fi

if prune && ! prune_sysroot "$OUTPUT_SYSROOT"; then
	echo "Error: failed to prune sysroot. Exiting." >&2
	cleanup_exit 1
fi

if ! link_lowercase "$OUTPUT_SYSROOT"; then
	echo "Error: failed to create lowercase symlinks to sysroot files. Exiting." >&2
	cleanup_exit 1
fi

# Link headers and libraries with casing provided by Microsoft to a special
# basename casing that is commonly used by codebases, and thus needs
# to exist in the sysroot.
link_special_case_sensitive "${SPECIAL_CASE_SENSITIVE_FILES[@]}"

# De-duplicate files in the sysroot, replacing duplicates with a link to
# the first file found with the same checksum.
# This handles cases where two files are named differently -
# e.g. with different casing, in the most common case -
# but contain the exact same contents, and thus only one file is required.
if ! dedup_dir "$OUTPUT_SYSROOT"; then
	echo "Error: failed to de-duplicate sysroot files. Exiting." >&2
	cleanup_exit 1
fi

cleanup_exit 0

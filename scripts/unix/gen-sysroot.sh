#!/usr/bin/env bash

set -euo pipefail

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

TEMP_COREUTILS_PATH=""
TEMP_COREUTILS_BIN=""

download_temp_coreutils() {
	echo "GNU coreutils missing. Attempting to download a temporary coreutils binary for your platform." 11>&2

	DOWNLOAD_DEPS=("jq" "curl" "tar" "cut" "sed" "unzip" "find")
	check_dependencies "${DOWNLOAD_DEPS[@]}"

	RUST_COREUTILS_ORG="uutils"
	RUST_COREUTILS_REPO="coreutils"
	RUST_COREUTILS_API_URI="https://api.github.com/repos/${RUST_COREUTILS_ORG}/${RUST_COREUTILS_REPO}/releases"

	SEARCH_HOST_TRIPLE="$(sed -e 's/-pc-/-unknown-/g' -e 's/-gnu$/-musl/' <<<"$HOST_TRIPLE")"

	latest_assets="$(curl -sSL -H "Accept: application/vnd.github+json" "$RUST_COREUTILS_API_URI" | jq -r '.[0].assets')"
	asset_count="$(jq -r 'length' <<<"$latest_assets")"

	platform_asset_name=""
	platform_asset_uri=""
	for ((i = 0; i < asset_count; i++)); do
		asset_name="$(jq -r ".[$i].name" <<<"$latest_assets")"
		asset_uri="$(jq -r ".[$i].browser_download_url" <<<"$latest_assets")"

		asset_triple="$(cut -d '-' -f3- <<<"$asset_name" | cut -d '.' -f1 | sed -e 's/-pc-/-unknown-/' -e 's/-apple-/-unknown-apple-/')"
		if [[ "$asset_triple" == "$SEARCH_HOST_TRIPLE" ]]; then
			platform_asset_name="$asset_name"
			platform_asset_uri="$asset_uri"
			break
		fi
	done

	if [ -n "$platform_asset_name" ] && [ -n "$platform_asset_uri" ]; then
		TEMP_ASSET_OUT="/tmp/${platform_asset_name}"
		if ! curl -sSL "$platform_asset_uri" -o "$TEMP_ASSET_OUT"; then
			echo "Error: GNU coreutils missing, and failed to download for your current platform. Install GNU coreutils, then try again." 1>&2
			rm "$TEMP_ASSET_OUT" 2>/dev/null || true
			exit 1
		fi

		TEMP_COREUTILS_PATH="${REPO_DIR}/build/coreutils"
		mkdir -p "$TEMP_COREUTILS_PATH"

		asset_ext="$(ext "$platform_asset_name" | to_lowercase)"
		if [[ "$asset_ext" == "zip" ]]; then
			if ! unzip "$TEMP_ASSET_OUT" -d "$TEMP_COREUTILS_PATH"; then
				echo "Error: GNU coreutils missing, and failed to extract download for your current platform. Install GNU coreutils, then try again." 1>&2
				rm "$TEMP_ASSET_OUT" 2>/dev/null || true
				exit 1
			fi
		elif [[ "$platform_asset_name" == *".tar."* ]]; then
			if ! tar -xvf "$TEMP_ASSET_OUT" -C "$TEMP_COREUTILS_PATH"; then
				echo "Error: GNU coreutils missing, and failed to extract download for your current platform. Install GNU coreutils, then try again." 1>&2
				rm "$TEMP_ASSET_OUT" 2>/dev/null || true
				exit 1
			fi
		fi

		rm "$TEMP_ASSET_OUT" 2>/dev/null || true
		TEMP_COREUTILS_BIN="$(find "$TEMP_COREUTILS_PATH" -type f -iname "coreutils" -o -iname "coreutils.exe" | head -n1)"

	else
		echo "Error: GNU coreutils missing, and could not find a download for your current platform. Install GNU coreutils, then try again." 1>&2
		exit 1
	fi
}

with_temp_coreutils() {
	if [ -z "$TEMP_COREUTILS_BIN" ]; then
		download_temp_coreutils
	fi

	local args
	args=()

	while [ "$#" -gt 0 ]; do
		args+=("$1")
		shift
	done

	"${TEMP_COREUTILS_BIN}" "${args[@]}"
}

cleanup_temp_coreutils() {
	if [ -n "$TEMP_COREUTILS_PATH" ] && [ -d "$TEMP_COREUTILS_PATH" ]; then
		rm -rf "$TEMP_COREUTILS_PATH"
	fi
}

# MacOS `sort` supports a smaller subset of options supported by GNU `sort`,
# so we try to find and use `gsort` instead if possible, and otherwise fallback to
# a temporary coreutils binary.
gnu_sort() {
	local args
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
	if [ -n "$target_dir" ]; then
		target_dir="$(normalize_path "$target_dir")"
		if [ -d "$target_dir" ]; then

			declare -A file_hashes

			# Find only regular files to prevent touching existing symlinks or directories
			while IFS=$'\n' read -r cmp_file; do
				if [ -n "$cmp_file" ] && [ -f "$cmp_file" ]; then
					hash="$(sha256sum "$cmp_file" | awk '{print $1}')"

					original="${file_hashes[$hash]}"
					if [ -n "$original" ]; then
						echo "Linking duplicate: $original -> $cmp_file"

						rm "$cmp_file"

						lnrs "$original" "$cmp_file"
					else
						file_hashes[$hash]="$cmp_file"
					fi
				fi
			done < <(find "$target_dir" -type f | gnu_sort -hbf)
		fi
	fi
}

unpack_vsix() {
	local vsix
	local sysroot_dirname
	local out_dir

	vsix="$(normalize_path "$1")"
	sysroot_dirname="$2"
	out_dir="$(normalize_path "$3")"

	tmp_out="${REPO_DIR}/build/.vsix-tmp"

	if [ -d "$tmp_out" ]; then
		rm -rf "$tmp_out"
	fi

	mkdir -p "$tmp_out"
	tmp_out_contents="${tmp_out}/Contents"

	local return_code=0
	if unzip "$vsix" -d "$tmp_out" && [ -d "$tmp_out_contents" ]; then
		while IFS=$'\n' read -r sysroot_dir; do
			if [ -n "$sysroot_dir" ] && [ -d "$sysroot_dir" ]; then
				arch_nested_sysroot_dir="${sysroot_dir}/x64"
				if [ -d "${arch_nested_sysroot_dir}" ]; then
					sysroot_dir="${arch_nested_sysroot_dir}"
				fi

				if ! rsync -avc --info=progress2 "${sysroot_dir}/" "${out_dir}/"; then
					return_code=1
				fi
			fi
		done < <(find "$tmp_out_contents" -type d -name "$sysroot_dirname")
	else
		return_code=1
	fi

	rm -rf "$tmp_out"
	return "$return_code"
}

unpack_atl() {
	return_code=0

	while IFS=$'\n' read -r atl_lib_vsix; do
		if ! unpack_vsix "$atl_lib_vsix" "lib" "${OUTPUT_SYSROOT}/lib"; then
			return_code=1
			break
		fi
	done < <(find "$XWIN_DL_DIR" -type f -name "Microsoft.VC.*ATL.X64.base.vsix")

	if [ $return_code -eq 0 ]; then
		while IFS=$'\n' read -r atl_header_vsix; do
			if ! unpack_vsix "$atl_header_vsix" "include" "${OUTPUT_SYSROOT}/include"; then
				return_code=1
				break
			fi
		done < <(find "$XWIN_DL_DIR" -type f -name "Microsoft.VC.*ATL.Headers.base.vsix")
	fi

	return $return_code
}

remove_intrinsic_headers() {
	while IFS=$'\n' read -r intrin; do
		if [ -n "$intrin" ]; then
			if [ -f "$intrin" ] || [ -L "$intrin" ]; then
				rm "$intrin"
			fi
		fi
	done < <(find "$OUTPUT_SYSROOT_INCLUDE_PATH" -iname "*intrin.h" -o -iname "intrin*.h")
}

link_lowercase() {
	local root
	root="$1"

	while IFS=$'\n' read -r tree_file; do
		if [ -n "$tree_file" ]; then
			lower_tree_file="$(dirname "$tree_file")/$(basename "$tree_file" | to_lowercase)"

			if [[ "$lower_tree_file" != "$tree_file" ]] &&
				! [ -e "$lower_tree_file" ] &&
				[ -e "$tree_file" ]; then
				echo "Linking lowercase: $tree_file -> $lower_tree_file"
				lnrs "$tree_file" "$lower_tree_file"
			fi
		fi
	done < <(find "$root" -type f -o -type l)

}

link_case_sensitive() {
	local lower
	local upper

	lower="$(normalize_path "$1")"
	upper="$(normalize_path "$2")"

	if [ -e "$lower" ] && ! [ -e "$upper" ]; then
		echo "Linking case sensitive: $lower -> $upper"
		lnrs "$lower" "$upper"
	elif ! [ -e "$lower" ] && [ -e "$upper" ]; then
		echo "Linking case sensitive: $upper -> $lower"
		lnrs "$upper" "$lower"
	fi
}

link_special_case_sensitive() {
	for cased in "${SPECIAL_CASE_SENSITIVE_FILES[@]}"; do
		lower="$(dirname "$cased")/$(basename "$cased" | to_lowercase)"
		link_case_sensitive "$lower" "$cased"
	done
}

REPO_DIR="$(repo_dir)"
. "${REPO_DIR}/scripts/unix/env.sh"

BUILD_DEPS=("rsync" "xwin" "sort" "unzip")
check_dependencies "${BUILD_DEPS[@]}"

HOST_TRIPLE="$(host_triple)"

XWIN_CACHE="${REPO_DIR}/build/xwin-cache"
mkdir -p "${XWIN_CACHE}"

OUTPUT_SYSROOT="$1"

[ -z "$OUTPUT_SYSROOT" ] && OUTPUT_SYSROOT="/opt/x86_64-unknown-windows-msvc"

OUTPUT_SYSROOT_INCLUDE_PATH="${OUTPUT_SYSROOT}/include"
OUTPUT_SYSROOT_LIB_PATH="${OUTPUT_SYSROOT}/include"

SPECIAL_CASE_SENSITIVE_FILES=(
	"${OUTPUT_SYSROOT_INCLUDE_PATH}/Windows.h"
	"${OUTPUT_SYSROOT_INCLUDE_PATH}/OAIdl.h"
	"${OUTPUT_SYSROOT_INCLUDE_PATH}/OCIdl.h"
	"${OUTPUT_SYSROOT_LIB_PATH}/Crypt32.lib"
	"${OUTPUT_SYSROOT_LIB_PATH}/Crypt32.Lib"
)

mkdir -p "$(dirname "$OUTPUT_SYSROOT")"

WINDOWS_MSVC_SYSROOT_REPO_URI="https://github.com/trcrsired/windows-msvc-sysroot"
mkdir -p "$OUTPUT_SYSROOT"

if ! xwin --accept-license \
	--cache-dir "$XWIN_CACHE" \
	--channel=release \
	--arch=x86_64 \
	--include-atl \
	--variant desktop \
	--include-debug-runtime \
	--http-retry 10 \
	--timeout 3600s \
	download; then
	echo "Error: xwin download failed. Exiting" 1>&2
	exit 1
fi

XWIN_SPLAT_DIR="${XWIN_CACHE}/splat"
XWIN_DL_DIR="${XWIN_CACHE}/dl"

if ! xwin --cache-dir "$XWIN_CACHE" \
	--accept-license \
	splat \
	--include-debug-libs \
	--include-debug-symbols || ! [ -d "$XWIN_SPLAT_DIR" ]; then
	echo "Error: xwin splat failed. Exiting." 1>&2
fi

RUNTIMES=(
	"shared"
	"cppwinrt"
	"ucrt"
	"um"
	"winrt"
	"" # None
)
SYSROOT_ARCH="x86_64"
SYSROOT_DIRNAMES=(
	"lib"
	"include"
)

while IFS=$'\n' read -r dist; do
	if [ -n "$dist" ] && [ -d "$dist" ]; then
		for sysroot_dirname in "${SYSROOT_DIRNAMES[@]}"; do
			found_runtime=0

			for rt in "${RUNTIMES[@]}"; do
				sysroot_dir="$(normalize_path "${dist}/${sysroot_dirname}/${rt}")"

				arch_nested="${sysroot_dir}/${SYSROOT_ARCH}"
				if [ -d "$arch_nested" ]; then
					sysroot_dir="${arch_nested}"
				fi

				if [ -d "$sysroot_dir" ]; then
					if [ -n "$rt" ]; then
						found_runtime=1
					elif [ $found_runtime -eq 1 ]; then
						# Directory structure uses runtime sub-roots.
						# Don't copy this directory containing each of them.
						continue
					fi

					out_sysroot_dir="${OUTPUT_SYSROOT}/${sysroot_dirname}"
					mkdir -p "$out_sysroot_dir"

					rsync -avc --info=progress2 "${sysroot_dir}/" "${out_sysroot_dir}/"
				fi
			done
		done
	fi
done < <(find "$XWIN_SPLAT_DIR" -mindepth 1 -maxdepth 1 -type d)

# `xwin` does not unpack the ATL VSIX archives - this is a bug.
# We can unpack these manually.
if ! unpack_atl; then
	echo "Error: Failed to unpack ATL VSIX archives. Exiting." 1>&2
	exit 1
fi

# These are provided by clang separately.
# Keeping these causes the compiler to use those provided by MSVC by default.
# This leads to definition conflicts, and thus compilation errors.
remove_intrinsic_headers

link_lowercase "$OUTPUT_SYSROOT"

# Link headers and libraries with casing provided by Microsoft to a special
# basename casing that is commonly used by codebases, and thus needs
# to exist in the sysroot.
link_special_case_sensitive

# De-duplicate files in the sysroot, replacing duplicates with a link to
# the first file found with the same checksum.
# This handles cases where two files are named differently -
# e.g. with different casing, in the most common case -
# but contain the exact same contents, and thus only one file is required.
dedup_dir "$OUTPUT_SYSROOT"

cleanup_temp_coreutils

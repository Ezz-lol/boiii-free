#!/usr/bin/env bash

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
	repo_dir="$(git -C "$(script_dir)" rev-parse --show-toplevel)"
	if ! realpath "$repo_dir" 2>/dev/null; then
		echo "$repo_dir"
	fi
}

. "$(repo_dir)/scripts/unix/env.sh"

REPO_DIR="$(repo_dir)"
BUILD_DIR="${REPO_DIR}/build"

SCRIPT_DIR="$(script_dir)"

BUILD_EXECUTOR="ninja"
CLEAN=0
RELEASE=0
OUTPUT_DIR=""
TIDY=0
EXEC_ARBITRARY=0
EXEC_ARGS=()
MARCH="x86-64"
NUM_THREADS="$(num_threads)"
BOIII_EXE="boiii.exe"
TLS_DLL="tlsdll.dll"
VERBOSE=0

# Enumeration-like encodings for possible configured build executor
BUILD_EXECUTOR_TYPE_GMAKE=0
BUILD_EXECUTOR_TYPE_NINJA=1
BUILD_EXECUTOR_TYPE_INVALID=-1

# Parses configured build executor name and returns equivalent build executor enumeration.
# Used to assign enumeration to global later, allowing usage in conditionals without the overhead of
# repetitive string parsing
build_executor_type() {
	case "$BUILD_EXECUTOR" in
	"ninja")
		echo "$BUILD_EXECUTOR_TYPE_NINJA"
		;;
	"gmake")
		echo "$BUILD_EXECUTOR_TYPE_GMAKE"
		;;
	*)
		echo "$BUILD_EXECUTOR_TYPE_INVALID"
		;;
	esac
}

exec_arbitrary() {
	[ "$EXEC_ARBITRARY" -ne 0 ]
}

tidy() {
	[ "$TIDY" -ne 0 ]
}

build_dir() {
	echo "$(repo_dir)/build"
}

build_type_dir() {
	if [ "$RELEASE" -eq 1 ]; then
		echo "Release"
	else
		echo "Debug"
	fi
}

output_path() {
	local path
	path=""
	if [ -n "$OUTPUT_DIR" ]; then
		path="${OUTPUT_DIR}/${1}"
	else
		path="$(build_dir)/bin/x64/$(build_type_dir)/$1"
	fi

	if ! [ -e "$path" ]; then
		echo "Error: Output file '$path' does not exist." >&2
		exit 1
	fi
	normalize_path "$path"
}

should_clean() {
	[ "$CLEAN" -ne 0 ]
}

BUILD_EXECUTOR_TYPE="$BUILD_EXECUTOR_TYPE_INVALID"

build_type() {
	if [ "$RELEASE" -eq 1 ]; then
		if [ "$BUILD_EXECUTOR_TYPE" -eq "$BUILD_EXECUTOR_TYPE_NINJA" ]; then
			echo "Release_x64"
		else
			echo "release_x64"
		fi
	else
		if [ "$BUILD_EXECUTOR_TYPE" -eq "$BUILD_EXECUTOR_TYPE_NINJA" ]; then
			echo "Debug_x64"
		else
			echo "debug_x64"
		fi
	fi
}

cc_default_defines() {
	local cc

	if [ "$#" -gt 0 ]; then
		cc="$1"
	fi

	if [ -z "$cc" ]; then
		if [ -n "$CC" ]; then
			cc="$(resolve_path "$CC")"
		else
			return 1
		fi
	fi

	"$cc" -E -dM - </dev/null

}

cc_is_gcc() {
	local defines
	defines="$(cc_default_defines "$@")" || return 1

	[[ "$defines" == *'__GNUC__'* ]]
}

cc_is_clang() {
	local defines
	defines="$(cc_default_defines "$@")" || return 1

	[[ "$defines" == *'__clang__'* ]]
}

get_clang() {
	if cc_is_clang "$CC"; then
		resolve_path "$CC"
	else
		resolve_path "clang"
	fi
}

get_llvm_bin() {
	"$(get_clang)" -### 2>&1 |
		grep 'InstalledDir:' |
		sed 's/^[ \t]*InstalledDir:[ \t]//' |
		normalize_path
}

get_llvm_ar() {
	if ! first_in_dir "$(get_llvm_bin)" "llvm-ar" "ar"; then
		echo "Error: Could not find llvm-ar or ar in LLVM bin directory: \"$(get_llvm_bin)\"." >&2
		exit 1
	fi
}

get_llvm_as() {
	if ! first_in_dir "$(get_llvm_bin)" "llvm-as" "as"; then
		echo "Error: Could not find llvm-as or as in LLVM bin directory: \"$(get_llvm_bin)\"." >&2
		exit 1
	fi
}

get_llvm_windres() {
	if ! first_in_dir "$(get_llvm_bin)" "llvm-windres" "windres"; then
		echo "Error: Could not find llvm-windres or windres in LLVM bin directory: \"$(get_llvm_bin)\"." >&2
		exit 1
	fi
}

get_llvm_strip() {
	if ! first_in_dir "$(get_llvm_bin)" "llvm-strip" "strip"; then
		echo "Error: Could not find llvm-strip or strip in LLVM bin directory: \"$(get_llvm_bin)\"." >&2
		exit 1
	fi
}

get_llvm_ld() {
	if ! first_in_dir "$(get_llvm_bin)" "ld.lld" "ld"; then
		echo "Error: Could not find ld.lld or ld in LLVM bin directory: \"$(get_llvm_bin)\"." >&2
		exit 1
	fi
}

get_llvm_lld() {
	if ! first_in_dir "$(get_llvm_bin)" "ld.lld" "lld"; then
		echo "Error: Could not find ld.lld or lld in LLVM bin directory: \"$(get_llvm_bin)\"." >&2
		exit 1
	fi
}

get_llvm_lld_link() {
	if ! first_in_dir "$(get_llvm_bin)" "ld.lld-link" "lld-link"; then
		echo "Error: Could not find ld.lld-link or lld-link in LLVM bin directory: \"$(get_llvm_bin)\"." >&2
		exit 1
	fi
}

get_llvm_clangpp() {
	if cc_is_clang "$CXX"; then
		resolve_path "$CXX"
	elif ! first_in_dir "$(get_llvm_bin)" "clang++" "clangpp"; then
		echo "Error: Could not find clang++ or clangpp in LLVM bin directory: \"$(get_llvm_bin)\"." >&2
		exit 1
	fi
}

get_llvm_objcopy() {
	if ! first_in_dir "$(get_llvm_bin)" "llvm-objcopy" "objcopy"; then
		echo "Error: Could not find llvm-objcopy or objcopy in LLVM bin directory: \"$(get_llvm_bin)\"." >&2
		exit 1
	fi
}

get_llvm_objdump() {
	if ! first_in_dir "$(get_llvm_bin)" "llvm-objdump" "objdump"; then
		echo "Error: Could not find llvm-objdump or objdump in LLVM bin directory: \"$(get_llvm_bin)\"." >&2
		exit 1
	fi
}

get_llvm_nm() {
	if ! first_in_dir "$(get_llvm_bin)" "llvm-nm" "nm"; then
		echo "Error: Could not find llvm-nm or nm in LLVM bin directory: \"$(get_llvm_bin)\"." >&2
		exit 1
	fi
}

get_llvm_readelf() {
	if ! first_in_dir "$(get_llvm_bin)" "llvm-readelf" "readelf"; then
		echo "Error: Could not find llvm-readelf or readelf in LLVM bin directory: \"$(get_llvm_bin)\"." >&2
		exit 1
	fi
}

get_llvm_size() {
	if ! first_in_dir "$(get_llvm_bin)" "llvm-size" "size"; then
		echo "Error: Could not find llvm-size or size in LLVM bin directory: \"$(get_llvm_bin)\"." >&2
		exit 1
	fi
}

get_llvm_dlltool() {
	if ! first_in_dir "$(get_llvm_bin)" "llvm-dlltool" "dlltool"; then
		echo "Error: Could not find llvm-dlltool or dlltool in LLVM bin directory: \"$(get_llvm_bin)\"." >&2
		exit 1
	fi
}

get_llvm_addr2line() {
	if ! first_in_dir "$(get_llvm_bin)" "llvm-addr2line" "addr2line"; then
		echo "Error: Could not find llvm-addr2line or addr2line in LLVM bin directory: \"$(get_llvm_bin)\"." >&2
		exit 1
	fi
}

get_llvm_coverage() {
	if ! first_in_dir "$(get_llvm_bin)" "llvm-cov" "cov"; then
		echo "Error: Could not find llvm-cov in LLVM bin directory: \"$(get_llvm_bin)\"." >&2
		exit 1
	fi
}

get_ld_mold() {
	resolve_path "ld.mold"
}

# shellcheck disable=SC2329
get_llvm_sysroot() {
	normalize_path "$(get_llvm_bin)/.."
}

get_clang_resource_dir() {
	normalize_path "$("$(get_clang)" -print-resource-dir)"
}

compiler_rt_include_paths() {
	normalize_path "$(get_clang_resource_dir)/include"
}

get_compiler_rt_path() {
	normalize_path "$("$(get_clang)" --print-libgcc-file-name -rtlib=compiler-rt)"
}

get_llvm_libdir() {
	realpath "$(get_llvm_sysroot)/lib"
}

cross_env() {
	local temp_windres_link_dir
	local temp_path

	local -a args
	args=()
	while [ "$#" -gt 0 ]; do
		args+=("$1")
		shift
	done

	# Place LLVM bin path before all others.
	# premake5 uses `windres` verbatim, with first resolved on path, to
	# compile the resources.
	# On most unix-like systems, this will not be LLVM windres, but GNU windres, which
	# does not support COFF relocations, and thus fails to compile the resources with errors.
	# By placing the LLVM bin directory path first, we ensure that premake5 finds the correct windres,
	# and thus can compile the resources successfully.
	temp_path="$(get_llvm_bin):${PATH}"
	# ensure LLVM windres with "windres" basename exists on path, somewhere.
	temp_windres_link_dir="$(mktemp -d)"
	trap 'rm -rf "$temp_windres_link_dir" >/dev/null 2>&1' EXIT INT TERM HUP

	if is_llvm "$(env PATH="$temp_path" resolve_path "windres" 2>/dev/null || true)"; then
		if [ -d "$temp_windres_link_dir" ]; then
			rm -rf "$temp_windres_link_dir" >/dev/null 2>&1 || true
			trap - EXIT INT TERM HUP
		fi
	else
		ln -s "$(get_llvm_windres)" "${temp_windres_link_dir}/windres"
		temp_path="${temp_windres_link_dir}:${temp_path}"
	fi

	exit_code=0
	if ! chdir "${REPO_DIR}" \
		env \
		PATH="${temp_path}" \
		CC="$TEMP_CC" \
		CXX="$TEMP_CXX" \
		CPP="$TEMP_CPP" \
		AS="$TEMP_AS" \
		AR="$TEMP_AR" \
		WINDRES="$TEMP_WINDRES" \
		STRIP="$TEMP_STRIP" \
		LD="$TEMP_LD" \
		OBJCOPY="$(get_llvm_objcopy)" \
		OBJDUMP="$(get_llvm_objdump)" \
		NM="$(get_llvm_nm)" \
		READELF="$(get_llvm_readelf)" \
		DLLTOOL="$(get_llvm_dlltool)" \
		ADDR2LINE="$(get_llvm_addr2line)" \
		COV="$(get_llvm_coverage)" \
		SIZE="$(get_llvm_size)" \
		CFLAGS="$TEMP_CFLAGS" \
		CXXFLAGS="$TEMP_CXXFLAGS" \
		LDFLAGS="$TEMP_LDFLAGS" \
		RESFLAGS="$TEMP_RESFLAGS" \
		RCFLAGS="$TEMP_RCFLAGS" \
		CMAKE_BUILD_PARALLEL_LEVEL="$NUM_THREADS" \
		MAKEOPTS="-j${NUM_THREADS}" \
		"${args[@]}"; then
		exit_code=1
	fi

	if [ -d "$temp_windres_link_dir" ]; then
		rm -rf "$temp_windres_link_dir" >/dev/null 2>&1 || true
	fi
	trap - EXIT INT TERM HUP

	return "$exit_code"
}

flags_to_yaml_array_items() {
	local flags_string="$1"
	IFS=$' ' read -r -a flags_array <<<"$flags_string"
	local num_flags="${#flags_array[@]}"
	for ((i = 0; i < $((num_flags - 1)); i++)); do
		if [ -n "${flags_array[i]}" ]; then
			echo -ne "${flags_array[i]},\n"
		fi
	done
	echo -ne "${flags_array[num_flags - 1]}"
}

FLAG_INDENTATION="   "
# shellcheck disable=SC2120
clangd_flag_indent() {
	local in

	if [ "$#" -gt 0 ]; then
		in="$1"
	else
		in="$(cat -)"
	fi

	# shellcheck disable=SC2001
	sed "s/^[ \t]*/${FLAG_INDENTATION}/g" <<<"$in"
}

SRC_INCLUDE_PATHS=(
	"src/common"
	"src/client"
)

DEP_INCLUDE_PATHS=(
	"deps"
	"deps/SteamworkSDK/public"
	"deps/argparse/include"
	"deps/frozen/include"
	"deps/Microsoft.Web.WebView2/build/native/include"
	"deps/curl/include"
	"deps/asmjit"
	"deps/imgui"
	"deps/discord-rpc/include"
	"deps/libtommath"
	"deps/libtomcrypt/src/headers"
	"deps/rapidjson/include"
	"deps/udis86"
	"deps/extra/udis86"
	"deps/extra/udis86/libudis86"
	"deps/minhook/include"
	"deps/stb"
	"deps/zlib"
	"deps/zlib/contrib/blast"
	"deps/zlib/contrib/infback9"
	"deps/zlib/contrib/iostream"
	"deps/zlib/contrib/iostream2"
	"deps/zlib/contrib/iostream3"
	"deps/zlib/contrib/minizip"
	"deps/zlib/contrib/puff"
	"deps/zstd/lib"
	"deps/brotli/c/include"
	"deps/gtl/include"
	"deps/hksc/src"
	"deps/opus/include"
	"deps/opus/silk"
	"deps/opus/silk/x86"
	"deps/opus/silk/float"
	"deps/opus/silk/float/x86"
	"deps/opus/celt"
	"deps/opus/celt/x86"
	"deps/portaudio/include"
	"deps/portaudio/src/common"
	"deps/portaudio/src/os/win"
	"deps/sqlite"
	"deps/sqlite_orm/include"
	"deps/xz/src/common"
	"deps/xz/src/liblzma/api"
	"deps/xz/src/liblzma/common"
	"deps/xz/src/liblzma/check"
	"deps/xz/src/liblzma/lz"
	"deps/xz/src/liblzma/lzma"
	"deps/xz/src/liblzma/rangecoder"
	"deps/xz/src/liblzma/delta"
	"deps/xz/src/liblzma/simple"
)

NUM_DEP_INCLUDES="${#DEP_INCLUDE_PATHS[@]}"

inc_path_flag() {
	local path
	path="$1"

	abs_path="${REPO_DIR}/${path}"
	echo "\"-I${abs_path}\",
\"-isystem=${abs_path}\"" | clangd_flag_indent
}

clangd_include_flags() {

	for src_inc in "${SRC_INCLUDE_PATHS[@]}"; do
		echo "$(inc_path_flag "$src_inc"),"
	done

	for ((dep_inc_idx = 0; dep_inc_idx < NUM_DEP_INCLUDES - 1; dep_inc_idx++)); do
		echo "$(inc_path_flag "${DEP_INCLUDE_PATHS[$dep_inc_idx]}"),"
	done

	inc_path_flag "${DEP_INCLUDE_PATHS[$((NUM_DEP_INCLUDES - 1))]}"

}

generate_clangd_config() {
	local clangd_config_path
	clangd_config_path="$(repo_dir)/.clangd"

	cat >"$clangd_config_path" <<EOL
CompileFlags:
  Add: [
$(flags_to_yaml_array_items "$TEMP_CXXFLAGS" | clangd_flag_indent), 
$(clangd_include_flags)
  ]
EOL
	echo "Generated clangd config: '$clangd_config_path'"
}

# Arguments optional, can use them later if needed
# shellcheck disable=SC2120
premake() {

	local -a args
	args=("--os=windows"
		"--cc=clang"
		"--shell=posix"
		"--arch=x86_64"
		"--file=${REPO_DIR}/premake5.lua"
	)
	if [ -n "$OUTPUT_DIR" ]; then
		args+=("--output=$OUTPUT_DIR")
	fi

	while [ "$#" -gt 0 ]; do
		args+=("$1")
		shift
	done

	args+=("$BUILD_EXECUTOR")
	cross_env \
		premake5 "${args[@]}"
}

execute_build() {

	local bash_path
	local -a BUILD_EXECUTOR_CMD

	bash_path="$(resolve_path "bash")"

	BUILD_EXECUTOR_CMD=("$BUILD_EXECUTOR")

	if [ "$BUILD_EXECUTOR_TYPE" -eq "$BUILD_EXECUTOR_TYPE_GMAKE" ]; then
		BUILD_EXECUTOR_CMD+=("SHELL=${bash_path}"
			"config=$(build_type)")
	elif [ "$BUILD_EXECUTOR_TYPE" -eq "$BUILD_EXECUTOR_TYPE_NINJA" ]; then
		BUILD_EXECUTOR_CMD+=("$(build_type)")
	fi

	BUILD_EXECUTOR_CMD+=(
		"-C${BUILD_DIR}"
		"-j${NUM_THREADS}")

	cross_env \
		env SHELL="${bash_path}" \
		"${BUILD_EXECUTOR_CMD[@]}"
}

print_usage() {
	echo "Cross-compilation build script for Windows using MSVC toolchain sysroot and LLVM toolchain on Unix-like systems."
	echo "Usage: $0 [OPTIONS]"
	echo "Options:"
	echo "  --exec, -e [command]       Execute an arbitrary command instead of building. Example: --exec echo 'Hello, world!'"
	echo "  --builder, -b [ninja|gmake]    Use the specified builder to execute the build configuration generated by premake (default: ninja)."
	echo "  --tidy, -t [tidy-args]     Run clang-tidy with all other arguments instead of building. Example: --tidy -checks='*' src/client/main.cpp"
	echo "  --clean, -c                Clean the build directory before building."
	echo "  --sysroot, -s              Specify the path to the MSVC toolchain sysroot."
	echo "  --release, -r              Build in release mode (default debug mode)."
	echo "  --output, -o               Specify the output directory for build files."
	echo "  -march, -m                 Specify the target architecture (default: x86-64-v2)."
	echo "  -j, --threads              Specify the number of threads to use for building (default: $(nproc))."
	echo "  -v, --verbose              Enable verbose compilation logging."
	echo "  --help, -h                 Show this help message and exit."
}

ORIG_ARGS=("$@")

while [ "$#" -gt 0 ]; do
	case "$1" in
	--clean | -clean | -c)
		CLEAN=1
		shift
		;;
	--sysroot | -sysroot | -s)
		if [ -z "$2" ]; then
			echo "Error: --sysroot requires an argument." >&2
			exit 1
		fi
		export WINDOWS_MSVC_SYSROOT="$2"
		shift 2
		;;
	--release | -release | -r)
		RELEASE=1
		shift
		;;
	--output | -output | -o)
		if [ -z "$2" ]; then
			echo "Error: --output requires an argument." >&2
			exit 1
		fi
		export OUTPUT_DIR="$2"
		shift 2
		;;
	--march | -march | -m)
		if [ -z "$2" ]; then
			echo "Error: --arch requires an argument." >&2
			exit 1
		fi
		MARCH="$2"
		shift 2
		;;
	--verbose | -verbose | -v)
		VERBOSE=1
		shift 1
		;;
	-j | -threads | --threads)
		if [ -z "$2" ]; then
			echo "Error: --threads requires an argument." >&2
			exit 1
		fi
		NUM_THREADS="$2"
		shift 2
		;;
	-j[0-9]*)
		NUM_THREADS="${1:2}"
		shift
		;;
	-threads=[0-9]* | --threads=[0-9]*)
		NUM_THREADS="$(cut -d '=' -f2 <<<"$1")"
		shift
		;;
	--tidy | -tidy | -t)
		TIDY=1
		shift
		;;
	--exec | -exec | -e)
		EXEC_ARBITRARY=1
		shift
		;;
	--builder | -b)
		BUILD_EXECUTOR="$2"
		shift 2
		;;
	--help | -help | -h | help)
		print_usage
		exit 0
		;;
	*)
		shift
		;;
	esac
done

BUILD_EXECUTOR_TYPE="$(build_executor_type)"

if tidy || exec_arbitrary; then
	# append the other arguments to EXEC_ARGS ; these were not meant for us.
	EXEC_ARGS=()
	found_arg=0

	for arg in "${ORIG_ARGS[@]}"; do
		case "$arg" in
		--exec | -exec | -e)

			if [ "$found_arg" -eq 0 ] && exec_arbitrary; then
				found_arg=1
			else
				EXEC_ARGS+=("$arg")
			fi
			;;
		--tidy | -tidy | -t)
			if [ "$found_arg" -eq 0 ] && tidy; then
				found_arg=1
			fi
			;;

		*)
			EXEC_ARGS+=("$arg")
			;;
		esac
	done
elif [ "${#EXEC_ARGS[@]}" -gt 0 ]; then
	echo "Error: unknown arguments found: ${EXEC_ARGS[*]}" >&2
	print_usage
	exit 1
fi

TARGET_TRIPLE="x86_64-unknown-windows-msvc"
DEFAULT_TOOLCHAIN_PATH="/opt/${TARGET_TRIPLE}"
[ -z "$WINDOWS_MSVC_SYSROOT" ] && [ -d "DEFAULT_TOOLCHAIN_PATH" ] && WINDOWS_MSVC_SYSROOT="${DEFAULT_TOOLCHAIN_PATH}"

if [ -z "$WINDOWS_MSVC_SYSROOT" ]; then
	echo "Error: WINDOWS_MSVC_SYSROOT environment variable is not set." >&2
	echo "This variable should point to the root of the MSVC toolchain sysroot, which contains the \
  'bin', 'lib', and 'include' directories for the MSVC toolchain." >&2
	echo "If you do not already have an ${TARGET_TRIPLE} sysroot, one can be generated using the script ${SCRIPT_DIR}/gen-sysroot.sh."
	exit 1
fi

msvc_toolchain_sysroot="$(normalize_path "$WINDOWS_MSVC_SYSROOT")"
if ! [ -d "$msvc_toolchain_sysroot" ]; then
	echo "Error: msvc_toolchain_sysroot path '$WINDOWS_MSVC_SYSROOT' does not exist or is not a directory." >&2
	exit 1
fi

WINDOWS_MSVC_TOOLCHAIN_BIN_PATH="${msvc_toolchain_sysroot}/bin"
WINDOWS_MSVC_TOOLCHAIN_LIB_PATH="${msvc_toolchain_sysroot}/lib"
# For structure used in https://github.com/trcrsired/windows-msvc-sysroot
WINDOWS_MSVC_TOOLCHAIN_LIB_PATH_NESTED="${msvc_toolchain_sysroot}/lib/${TARGET_TRIPLE}"
WINDOWS_MSVC_TOOLCHAIN_INCLUDE_PATH="${msvc_toolchain_sysroot}/include"

cflags=(
	"-isystem" "${WINDOWS_MSVC_TOOLCHAIN_INCLUDE_PATH}"
	"-fms-extensions"
	"-fuse-ld=lld"
	"--target=x86_64-windows-msvc"
	"-L${WINDOWS_MSVC_TOOLCHAIN_LIB_PATH}"
	"-L${WINDOWS_MSVC_TOOLCHAIN_BIN_PATH}"
	"-I${WINDOWS_MSVC_TOOLCHAIN_INCLUDE_PATH}")
cxxflags=(
	"-std=c++20"
	"-isystem" "${WINDOWS_MSVC_TOOLCHAIN_INCLUDE_PATH}"
	"-fms-extensions"
	"-fuse-ld=lld"
	"--target=x86_64-windows-msvc"
	"-I${WINDOWS_MSVC_TOOLCHAIN_INCLUDE_PATH}")
ldflags=("-static"
	"-fms-extensions"
	"-isystem"
	"${WINDOWS_MSVC_TOOLCHAIN_INCLUDE_PATH}"
	"-Wl,-libpath:${WINDOWS_MSVC_TOOLCHAIN_LIB_PATH}"
	"-Wl,-libpath:${WINDOWS_MSVC_TOOLCHAIN_BIN_PATH}"
	"-fuse-ld=lld"
	"--target=x86_64-windows-msvc"
	"-L${WINDOWS_MSVC_TOOLCHAIN_LIB_PATH}"
	"-L${WINDOWS_MSVC_TOOLCHAIN_BIN_PATH}"
	"-Wl,/subsystem:windows")

if [ -d "$WINDOWS_MSVC_TOOLCHAIN_LIB_PATH_NESTED" ] || [ -L "$WINDOWS_MSVC_TOOLCHAIN_LIB_PATH_NESTED" ]; then
	cflags+=("-L${WINDOWS_MSVC_TOOLCHAIN_LIB_PATH_NESTED}")
	ldflags+=("-Wl,-libpath:${WINDOWS_MSVC_TOOLCHAIN_LIB_PATH_NESTED}"
		"-L${WINDOWS_MSVC_TOOLCHAIN_LIB_PATH_NESTED}")
fi

if [ "$RELEASE" -eq 1 ]; then
	ldflags+=("-Wl,/release")
fi

resflags=("--target=pe-x86-64" "-I${msvc_toolchain_sysroot}/include")

if [ "$VERBOSE" -eq 1 ]; then
	ldflags+=("-v")
	cxxflags+=("-v")
	cflags+=("-v")
	resflags+=("--verbose")
fi

disabled_warnings=(
	"unknown-warning-option"
	"microsoft-cast"
	"dangling-else"
	"nonportable-include-path"
	"error=implicit-function-declaration"
	"error=microsoft-case"
	"error=dangling-else"
	"system-headers"
	"sign-compare"
	"format"
	"unused-command-line-argument"
)

for warning in "${disabled_warnings[@]}"; do
	cflags+=("-Wno-$warning" "-Wno-error=$warning" "-Wno-clang-diagnostic-$warning" "-Wno-error=clang-diagnostic-$warning")
	cxxflags+=("-Wno-$warning" "-Wno-error=$warning" "-Wno-clang-diagnostic-$warning" "-Wno-error=clang-diagnostic-$warning")
	ldflags+=("-Wno-$warning" "-Wno-error=$warning" "-Wno-clang-diagnostic-$warning" "-Wno-error=clang-diagnostic-$warning")
done

TEMP_CFLAGS="${cflags[*]}"
TEMP_CXXFLAGS="${cxxflags[*]}"
TEMP_LDFLAGS="${ldflags[*]}"
TEMP_RESFLAGS="${resflags[*]}"
TEMP_RCFLAGS="$TEMP_CXXFLAGS"
TEMP_CC="$(get_clang)"
TEMP_CXX="$(get_llvm_clangpp)"
TEMP_CPP="$(get_llvm_clangpp)"
TEMP_AS="$(get_llvm_as)"
TEMP_AR="$(get_llvm_ar)"
TEMP_WINDRES="$(get_llvm_windres)"
TEMP_STRIP="$(get_llvm_strip)"
TEMP_LD="$(get_llvm_ld)"

BUILD_DEPS=(
	"$BUILD_EXECUTOR"
	"premake5"
	"env"
	"grep"
	"sed"
)

if tidy; then
	if ! cross_env \
		clang-tidy "${EXEC_ARGS[@]}" -- \
		-I"${WINDOWS_MSVC_TOOLCHAIN_INCLUDE_PATH}"; then
		exit 1
	fi

elif exec_arbitrary; then
	# In case the command is opening an editor or an application that uses clangd otherwise
	generate_clangd_config
	if ! cross_env \
		"${EXEC_ARGS[@]}"; then
		exit 1
	fi
else

	check_dependencies "${BUILD_DEPS[@]}"
	if should_clean && [ -d "$BUILD_DIR" ]; then
		rm -rf "$BUILD_DIR" 2>/dev/null || true
	fi

	if ! premake; then
		echo "Error: Premake failed." >&2
		exit 1
	fi

	if execute_build; then
		echo "Build completed successfully."
		echo "${BOIII_EXE} -> $(output_path "$BOIII_EXE")"
		echo "${TLS_DLL} -> $(output_path "$TLS_DLL")"
	else
		echo "Error: Build failed." >&2
		exit 1
	fi
fi

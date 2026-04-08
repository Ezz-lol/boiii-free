#!/usr/bin/env bash

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

CLEAN=0
RELEASE=0
OUTPUT_DIR=""
TIDY=0
EXEC_ARBITRARY=0
EXEC_ARGS=()
MARCH="x86-64-v2"
NUM_THREADS="$(nproc)"
BOIII_EXE="boiii.exe"
TLS_DLL="tlsdll.dll"

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
    path="${OUTPUT_DIR}/$1"
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

build_type() {
  if [ "$RELEASE" -eq 1 ]; then
    echo "release_x64"
  else
    echo "debug_x64"
  fi
}

get_clang() {
  resolve_path "clang"
}

get_llvm_bin() {
  "$(get_clang)" -### 2>&1 |
    grep 'InstalledDir:' |
    sed 's/.*InstalledDir:\s*//' |
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
  if ! first_in_dir "$(get_llvm_bin)" "clang++" "clangpp"; then
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
  if ! first_in_dir "$(get_llvm_bin)" "llvm-cov" "llvm-cov"; then
    echo "Error: Could not find llvm-cov in LLVM bin directory: \"$(get_llvm_bin)\"." >&2
    exit 1
  fi
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

windres_is_llvm() {
  local windres_path
  windres_path="$1"
  if grep -q 'LLVM windres' <<<"$("${windres_path}" --version 2>&1)"; then
    return 0
  else
    return 1
  fi
}

cross_env() {

  local args
  args=()
  while [ "$#" -gt 0 ]; do
    args+=("$1")
    shift
  done

  # Put LLVM bin path before all others.
  # premake5 uses `windres` verbatim, with first resolved on path, to
  # compile the resources.
  # On most unix-like systems, this will not be LLVM windres, but GNU windres, which
  # does not support COFF relocations, and thus fails to compile the resources with errors.
  # By putting LLVM bin path first, we ensure that premake5 finds the correct windres, and thus can compile the resources successfully.
  TEMP_PATH="$(get_llvm_bin):${PATH}"
  # ensure literal "windres" file for LLVM windres exists on path, somewhere.
  temp_windres_link_dir="$(mktemp -d)"
  resolved_windres="$(env PATH="$TEMP_PATH" which windres | normalize_path)"

  if ! windres_is_llvm "$resolved_windres"; then
    ln -s "$(get_llvm_windres)" "${temp_windres_link_dir}/windres"
    TEMP_PATH="${temp_windres_link_dir}:${TEMP_PATH}"

  fi
  exit_code=0
  if ! env --chdir="${REPO_DIR}" \
    env PATH="$(get_llvm_bin):${PATH}" \
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
    "${args[@]}"; then
    exit_code=1
  fi

  if [ -d "$temp_windres_link_dir" ]; then
    rm -rf "$temp_windres_link_dir" 2>&1 || true
  fi

  return "$exit_code"
}

# Arguments optional, can use them later if needed
# shellcheck disable=SC2120
premake() {

  local args
  args=("--os=windows"
    "--cc=clang"
    "--shell=posix"
    --arch="x86_64"
    "--file=${REPO_DIR}/premake5.lua"
  )
  if [ -n "$OUTPUT_DIR" ]; then
    args+=("--output=$OUTPUT_DIR")
  fi

  while [ "$#" -gt 0 ]; do
    args+=("$1")
  done

  args+=("gmake")
  cross_env \
    premake5 "${args[@]}"
}

link_capitalized_headers() {

  local needs_capitalized

  # All others have been enforced to use lowercase search path in codebase, but imgui uses "Windows.h" explicitly.
  # This is the only case-sensitive header lookup in our dependencies.
  needs_capitalized=("windows.h")

  for header in "${needs_capitalized[@]}"; do

    find_capitalized="$(find "${WINDOWS_MSVC_TOOLCHAIN_INCLUDE_PATH}")"
    if [ -z "$find_capitalized" ]; then
      find_case_insensitive="$(find "${WINDOWS_MSVC_TOOLCHAIN_INCLUDE_PATH}" -iname "$header")"
      if [ -n "$find_case_insensitive" ]; then
        find_dir="$(dirname "$find_case_insensitive")"
        echo "Linking ${find_case_insensitive} -> ${find_dir}/${header}"
        if ! ln -s "$find_case_insensitive" "${find_dir}/${header}"; then
          echo "Error: Failed to link ${find_case_insensitive} to ${find_dir}/${header}" >&2
          return 1
        fi

      else
        echo "Error: Could not find required header '$header' (case-insensitive) in MSVC toolchain include path: '${WINDOWS_MSVC_TOOLCHAIN_INCLUDE_PATH}')." >&2
        return 1
      fi
    fi

  done

  return 0
}

print_usage() {
  echo "Cross-compilation build script for Windows using MSVC toolchain sysroot and LLVM toolchain on Unix-like systems."
  echo "Usage: $0 [OPTIONS]"
  echo "Options:"
  echo " --exec, -e [command]        Execute an arbitrary command instead of building. Example: --exec echo Hello, world!"
  echo "  --tidy, -t [tidy-args]     Run clang-tidy with all other arguments instead of building. Example: --tidy -checks='*' src/client/main.cpp"
  echo "  --clean, -c                Clean the build directory before building."
  echo "  --sysroot, -s              Specify the path to the MSVC toolchain sysroot."
  echo "  --release, -r              Build in release mode (default debug mode)."
  echo "  --output, -o               Specify the output directory for build files."
  echo "  -march, -m                 Specify the target architecture (default: x86-64-v2)."
  echo "  -j, --threads              Specify the number of threads to use for building (default: $(nproc))."
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
  -j | -threads | --threads)
    if [ -z "$2" ]; then
      echo "Error: --threads requires an argument." >&2
      exit 1
    fi
    NUM_THREADS="$2"
    shift 2
    ;;
  --tidy | -tidy | -t)
    TIDY=1
    shift
    ;;
  --exec | -exec | -e)
    EXEC_ARBITRARY=1
    shift
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

. "$(repo_dir)/scripts/unix/env.sh"
REPO_DIR="$(repo_dir)"
BUILD_DIR="${REPO_DIR}/build"

if [ -z "$WINDOWS_MSVC_SYSROOT" ]; then
  echo "Error: WINDOWS_MSVC_SYSROOT environment variable is not set." >&2
  echo "This variable should point to the root of the MSVC toolchain sysroot, which contains the \
  'bin', 'lib', and 'include' directories for the MSVC toolchain." >&2
  echo "You may create this yourself using MSSTL (C++ STL) and xwin (Windows CRT and SDK), or use a preconfigured sysroot with compatible structure, \
 such as that provided in the Github repository https://github.com/trcrsired/windows-msvc-sysroot.
  Note that you will need to remove all SSE instrinsic headers (\`rm \${SYSROOT}/include/**/*intrin*\`) from the sysroot, if any, to ensure that they do not 
  override the built-in clang intrinsic headers." >&2
  exit 1
fi

msvc_toolchain_sysroot="$(normalize_path "$WINDOWS_MSVC_SYSROOT")"
if ! [ -d "$msvc_toolchain_sysroot" ]; then
  echo "Error: msvc_toolchain_sysroot path '$WINDOWS_MSVC_SYSROOT' does not exist or is not a directory." >&2
  exit 1
fi

WINDOWS_MSVC_TOOLCHAIN_BIN_PATH="${msvc_toolchain_sysroot}/bin/x86_64-unknown-windows-msvc"
WINDOWS_MSVC_TOOLCHAIN_LIB_PATH="${msvc_toolchain_sysroot}/lib/x86_64-unknown-windows-msvc"
WINDOWS_MSVC_TOOLCHAIN_INCLUDE_PATH="${msvc_toolchain_sysroot}/include"

cflags=(
  "-Oz"
  "-msse" "-msse2" "-msse3" "-mssse3" "-msse4.1" "-msse4.2" "-mavx" "-mavx2" "-maes"
  "-march=${MARCH}" "-m64"
  "-isystem" "${WINDOWS_MSVC_TOOLCHAIN_INCLUDE_PATH}"
  "-fms-extensions"
  "-fuse-ld=lld"
  "--verbose"
  "--target=x86_64-windows-msvc"
  "-L${WINDOWS_MSVC_TOOLCHAIN_LIB_PATH}"
  "-L${WINDOWS_MSVC_TOOLCHAIN_BIN_PATH}"
  "-I${WINDOWS_MSVC_TOOLCHAIN_INCLUDE_PATH}")
cxxflags=(
  "-Oz"
  "-msse" "-msse2" "-msse3" "-mssse3" "-msse4.1" "-msse4.2" "-mavx" "-mavx2" "-maes"
  "-march=${MARCH}" "-m64"
  "-isystem" "${WINDOWS_MSVC_TOOLCHAIN_INCLUDE_PATH}/c++/msstl/"
  "-isystem" "${WINDOWS_MSVC_TOOLCHAIN_INCLUDE_PATH}"
  "-fms-extensions"
  "-fuse-ld=lld"
  "--verbose"
  "--target=x86_64-windows-msvc"
  "-I${WINDOWS_MSVC_TOOLCHAIN_INCLUDE_PATH}")
ldflags=("-static"
  "-march=${MARCH}" "-m64"
  "-fms-extensions"
  "-isystem"
  "${WINDOWS_MSVC_TOOLCHAIN_INCLUDE_PATH}"
  "-Wl,-libpath:${WINDOWS_MSVC_TOOLCHAIN_LIB_PATH}"
  "-Wl,-libpath:${WINDOWS_MSVC_TOOLCHAIN_BIN_PATH}"
  "-fuse-ld=lld"
  "--verbose"
  "--target=x86_64-windows-msvc"
  "-L${WINDOWS_MSVC_TOOLCHAIN_LIB_PATH}"
  "-L${WINDOWS_MSVC_TOOLCHAIN_BIN_PATH}"
  "-Wl,/subsystem:windows")
if [ "$RELEASE" -eq 1 ]; then
  ldflags+=("-Wl,/release")
fi

resflags=("-I${msvc_toolchain_sysroot}/include")

disabled_warnings=(
  "unknown-warning-option"
  "microsoft-cast"
  "dangling-else"
  "nonportable-include-path"
  "error=implicit-function-declaration"
  "error=microsoft-case"
  "error=dangling-else"
  "system-headers"
  "unused-function"
  "unused-variable"
  "sign-compare"
  "unused-but-set-variable"
  "format"
  "unused-lambda-capture"
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
  "$TEMP_CC"
  "$TEMP_CXX"
  "$TEMP_CPP"
  "$TEMP_AS"
  "$TEMP_AR"
  "$TEMP_WINDRES"
  "windres"
  "$TEMP_STRIP"
  "$TEMP_LD"
  "gmake"
  "premake5"
)

if tidy; then

  if ! cross_env \
    clang-tidy "${EXEC_ARGS[@]}" -- \
    -I"${WINDOWS_MSVC_TOOLCHAIN_INCLUDE_PATH}" \
    -I"${WINDOWS_MSVC_TOOLCHAIN_INCLUDE_PATH}/c++/msstl/"; then
    exit 1
  fi

elif exec_arbitrary; then
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

  if ! link_capitalized_headers; then
    echo "Error: Failed to link capitalized headers." >&2
    exit 1
  fi

  bash_path="$(resolve_path "bash")"
  if cross_env \
    env SHELL="${bash_path}" \
    gmake SHELL="${bash_path}" \
    config="$(build_type)" \
    -Cbuild \
    -j"$NUM_THREADS" \
    -e; then

    echo "Build completed successfully."
    echo "${BOIII_EXE} -> $(output_path "$BOIII_EXE")"
    echo "${TLS_DLL} -> $(output_path "$TLS_DLL")"
  else
    echo "Error: Build failed." >&2
    exit 1
  fi
fi

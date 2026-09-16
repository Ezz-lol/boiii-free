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

check_dependencies() {
	local deps=("git" "clang-format" "stylua" "shfmt" "yarn" "dos2unix" "kill")
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

REPO_DIR="$(repo_dir)"
. "${REPO_DIR}/scripts/unix/env.sh"

MAX_PARALLEL_JOBS=$(num_threads)

JOBS=()

queue_job() {
	args=()
	while [ "$#" -gt 0 ]; do
		args+=("$1")
		shift
	done

	job_loaded=0
	while [ $job_loaded -eq 0 ]; do
		if [ ${#JOBS[@]} -ge $MAX_PARALLEL_JOBS ]; then
			wait -n
			for i in "${!JOBS[@]}"; do
				if ! kill -0 "${JOBS[i]}" 2>/dev/null; then
					"${args[@]}" &
					JOBS[i]=$!
					job_loaded=1
					break
				fi
			done
		else
			"${args[@]}" &
			JOBS+=($!)
			job_loaded=1
		fi
	done
}

any_jobs_running() {
	for job in "${JOBS[@]}"; do
		if kill -0 "$job" 2>/dev/null; then
			return 0
		fi
	done
	return 1
}

wait_all_jobs() {
	while any_jobs_running; do
		for job in "${JOBS[@]}"; do
			if kill -0 "$job" 2>/dev/null; then
				wait "$job"
			fi
		done
	done
}

kill_all_jobs() {
	while any_jobs_running; do
		for job in "${JOBS[@]}"; do
			if kill -0 "$job" 2>/dev/null; then
				kill -9 "$job" 2>/dev/null || continue
			fi
		done
	done
}

trap 'kill_all_jobs' EXIT SIGINT SIGTERM SIGQUIT SIGTSTP

check_dependencies

if ! chdir "${REPO_DIR}" yarn install; then
	echo "Error: Failed to install \`yarn\` dependencies. Exiting." >&2
	exit 1
fi

format_file() {
	local path
	path="$1"

	if [ -n "$path" ] && [ -f "$path" ]; then
		case "$(ext "$path" | to_lowercase)" in
		"lua")
			if ! chdir "${REPO_DIR}" stylua "$path" || ! chdir "$REPO_DIR" dos2unix --quiet "$path"; then
				return 1
			fi
			;;
		"md" | "markdown" | "html" | "htm" | "js" | \
			"jsx" | "ts" | "tsx" | "mjs" | "cjs" | \
			"mts" | "cts" | "graphql" | "gql" | "json" | \
			"json5" | "webmanifest" | "yaml" | "yml" | "vue" | \
			"svelte" | "handlebars" | "hbs" | "toml" | "ps1")
			if ! chdir "${REPO_DIR}" yarn run prettier --config "${REPO_DIR}/.prettierrc.json" --write "$path" || ! chdir "$REPO_DIR" dos2unix --quiet "$path"; then
				return 1
			fi
			;;
		"sh")
			chdir "${REPO_DIR}" shfmt --write "$path"
			;;
		"c" | "cc" | \
			"cpp" | "cxx" | \
			"hpp" | "h" | \
			"hxx" | "js" | \
			"json" | "proto" | \
			"java" | "cs" | \
			"objc" | "sv" | \
			"svh" | "v" | \
			"svp" | "svl" | \
			"vlg" | "vt" | \
			"vh" | "sv" | \
			"verilog" | "vo" | \
			"vqm" | "vcd")
			if ! chdir "${REPO_DIR}" clang-format -i --style=file "$path" || ! chdir "$REPO_DIR" dos2unix --quiet "$path"; then
				return 1
			fi
			;;
		esac
	fi
}

should_format() {
	local path
	path="$1"

	if [ -n "$path" ]; then
		if ! [[ "$rel_src_file" =~ ^"deps/" ]] || [[ "$rel_src_file" =~ ^"deps/premake/" ]]; then
			return 0
		fi
	fi

	return 1
}

exit_code=0
while IFS=$'\n' read -r rel_src_file; do
	if should_format "$rel_src_file"; then
		src_file="$(normalize_path "${REPO_DIR}/${rel_src_file}")"
		if [ -n "$src_file" ] && [ -f "$src_file" ]; then
			queue_job format_file "$src_file"
		fi
	fi
done < <(chdir "$REPO_DIR" git ls-files --deduplicate --exclude-standard)

wait_all_jobs

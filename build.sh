#!/usr/bin/env bash

SCRIPT_DIR="$(realpath "$(dirname "$0")")"
. "${SCRIPT_DIR}/scripts/unix/env.sh"

DOCKER_CMD=()

if have_application "nerdctl"; then
	DOCKER_CMD+=("nerdctl")
elif have_application "docker"; then
	DOCKER_CMD+=("docker")
else
	echo "No viable Docker container engine found. Install either docker or nerdctl and containerd, then try again." >&2
	exit 1
fi

DOCKER_CMD+=("compose"
	"run"
	"--rm"
	"--build"
	"release"
)

"${DOCKER_CMD[@]}"

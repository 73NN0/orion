#!/usr/bin/env sh

set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
LOG=$(mktemp)

trap 'rm -f "$LOG"' EXIT HUP INT TERM

if (
	cd "$SCRIPT_DIR/engine"

	cmake -S . -B build \
		-DCMAKE_BUILD_TYPE=Debug \
		-DBUILD_STANDALONE=ON

	cmake --build build \
		--target uiqvm_reach \
		--verbose \
		--parallel 4
) >"$LOG" 2>&1
then
	status=0
else
	status=$?
fi

cat "$LOG"
xclip -selection clipboard <"$LOG"

exit "$status"
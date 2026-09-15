#!/bin/sh

set -eu

ENGINE_DIR="engine"
PATCH_DIR="patches"

IOQ3_REPO="https://github.com/ioquake/ioq3.git"
IOQ3_COMMIT="588393618dbc82e7207c21c6ddecca229944a03a"

if [ -e "$ENGINE_DIR" ]; then
	printf '%s\n' "error: $ENGINE_DIR already exists" >&2
	exit 1
fi

printf '%s\n' "cloning ioquake3..."
git clone "$IOQ3_REPO" "$ENGINE_DIR"

cd "$ENGINE_DIR"

printf '%s\n' "checking out $IOQ3_COMMIT..."
git checkout "$IOQ3_COMMIT"

printf '%s\n' "applying patches..."

for patchfile in "../$PATCH_DIR"/*.patch
do
	[ -e "$patchfile" ] || continue

	printf '  %s\n' "$patchfile"
	patch -p1 < "$patchfile"
done

printf '%s\n' "done"
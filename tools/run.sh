#!/bin/sh
# run.sh [carte] : lance Orion directement sur une carte.
#
#   tools/run.sh orion_niveau1
#
# Les réglages (sv_pure, bot_enable, touches) viennent de reach/default.cfg.

set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BIN=$ROOT/engine/build/Debug

[ -x "$BIN/orion" ] || {
	echo "run.sh: $BIN/orion n'existe pas : lancer ./compile.sh" >&2
	exit 1
}
cd "$BIN"
exec ./orion +devmap "${1:-orion_test}"

#!/bin/sh
# scenario.sh <carte> <scenario> : rejoue reach/scenarios/<scenario>.cfg
# dans le vrai client, sans écran (Xvfb), et affiche les positions lues.
#
#   tools/scenario.sh orion_niveau1 niveau1
#
# Un scénario est un fichier de commandes : il déplace le joueur
# (+forward, setviewpos), attend (wait), note des positions (viewpos),
# prend des captures (screenshot) et finit par quit. Rien n'est
# simulé : c'est le client, le serveur et les modules habituels.
#
# Sortie : une ligne « (x y z) : angle » par viewpos, puis le dossier
# des captures. Dépendance : Xvfb.

set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BIN=$ROOT/engine/build/Debug
map=${1:?usage: tools/scenario.sh <carte> <scenario>}
scen=${2:?usage: tools/scenario.sh <carte> <scenario>}
DPY=:82
OUT=$(mktemp -d)
XPID=

trap '[ -n "$XPID" ] && kill "$XPID" 2>/dev/null || :' EXIT

[ -x "$BIN/orion" ] || {
	echo "scenario.sh: $BIN/orion absent : lancer ./compile.sh" >&2
	exit 1
}
[ -f "$BIN/reach/scenarios/$scen.cfg" ] || {
	echo "scenario.sh: reach/scenarios/$scen.cfg absent du build : relancer ./compile.sh" >&2
	exit 1
}

Xvfb "$DPY" -screen 0 1024x768x24 </dev/null >/dev/null 2>&1 &
XPID=$!
sleep 1

# HOME temporaire : aucun réglage personnel ne fausse le test.
# s_initsound 0 : pas de carte son dans le test.
(cd "$BIN" && HOME=$OUT DISPLAY=$DPY timeout 120 ./orion \
	+set r_fullscreen 0 +set r_mode -1 \
	+set r_customwidth 960 +set r_customheight 540 \
	+set s_initsound 0 +set sv_pure 0 +set bot_enable 0 \
	+devmap "$map" +exec "scenarios/$scen") >"$OUT/journal" 2>&1 || :

grep -E '^\(-?[0-9. ]+\) : ' "$OUT/journal" || echo "scenario.sh: aucune position lue" >&2
echo "captures : $OUT/.local/share/orion/reach/screenshots"
echo "journal  : $OUT/journal"

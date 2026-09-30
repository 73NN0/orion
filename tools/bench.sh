#!/bin/sh
# bench.sh - teste l'UI sans écran : Xvfb, xdotool, clavier seulement.
#
#   ./compile.sh && tools/bench.sh
#
# Deux lancements du client, chacun depuis un menu neuf :
#   1. START (Bas, Entrée) doit envoyer « map orion_test » : le journal
#      contient alors « Server: orion_test » (la carte existe) ou, si elle
#      manque, « Can't find map maps/orion_test.bsp ».
#   2. QUIT (Bas, Bas, Entrée) doit fermer le client.
#
# Dépendances : Xvfb, xdotool.

set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BIN=$ROOT/engine/build/Debug
OUT=$(mktemp -d)
DPY=:81
XPID=
OPID=

cleanup()
{
	[ -n "$OPID" ] && kill "$OPID" 2>/dev/null || :
	[ -n "$XPID" ] && kill "$XPID" 2>/dev/null || :
}
trap cleanup EXIT

# run_client <journal> <touches...> : lance le client, attend l'UI,
# tape les touches, puis laisse 3 s au client pour réagir.
run_client()
{
	log=$1
	shift
	# HOME temporaire : aucun orion.cfg personnel ne fausse le test.
	(cd "$BIN" && HOME=$OUT DISPLAY=$DPY exec ./orion \
		+set r_fullscreen 0 +set r_mode -1 \
		+set r_customwidth 960 +set r_customheight 540 \
		+set s_initsound 0 +set sv_pure 0 +set bot_enable 0) >"$log" 2>&1 &
	OPID=$!
	sleep 9			# chargement du renderer et de l'UI
	DISPLAY=$DPY xdotool key "$@"
	sleep 3
}

# stop_client : 0 si le client s'était déjà arrêté seul, 1 sinon.
stop_client()
{
	alive=0
	kill -0 "$OPID" 2>/dev/null && alive=1 && kill "$OPID"
	wait "$OPID" 2>/dev/null || :	# journal complet une fois arrêté
	OPID=
	[ "$alive" -eq 0 ]
}

if [ ! -x "$BIN/orion" ]; then
	echo "$BIN/orion absent : lance d'abord ./compile.sh" >&2
	exit 1
fi

Xvfb "$DPY" -screen 0 1024x600x24 </dev/null >/dev/null 2>&1 &
XPID=$!
sleep 1

run_client "$OUT/start.log" Down Return
stop_client || :
start=ÉCHEC
grep -qE "maps/orion_test.bsp|Server: orion_test" "$OUT/start.log" && start=ok

run_client "$OUT/quit.log" Down Down Return
quit=ÉCHEC
stop_client && quit=ok

echo "START -> commande map : $start"
echo "QUIT  -> client fermé  : $quit"
echo "journaux : $OUT"
[ "$start" = ok ] && [ "$quit" = ok ]

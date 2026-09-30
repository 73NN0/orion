#!/bin/sh
# map.sh <nom> : compile maps-src/<nom>.map en reach/maps/<nom>.bsp
#
#   tools/map.sh orion_test
#
# Trois passes de q3map2 : géométrie (-meta), visibilité (-vis),
# lumière (-light). Le compilateur se trouve par PATH ; pour en
# utiliser un autre : Q3MAP2=/chemin/q3map2 tools/map.sh orion_test
#
# Rien n'est écrit dans maps-src/ : tout se passe dans un dossier
# temporaire, et seul le .bsp final est copié dans reach/maps/.

set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
Q3MAP2=${Q3MAP2:-q3map2}
name=${1:?usage: tools/map.sh <nom>}
src=$ROOT/maps-src/$name.map

command -v "$Q3MAP2" >/dev/null 2>&1 || {
	echo "map.sh: $Q3MAP2 introuvable (installer NetRadiant, ou Q3MAP2=...)" >&2
	exit 1
}
[ -f "$src" ] || {
	echo "map.sh: $src n'existe pas" >&2
	exit 1
}

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
cp "$src" "$work/$name.map"
mkdir -p "$ROOT/reach/maps"

# -fs_basepath/-fs_game : q3map2 cherche les textures et les .shader
# dans reach/, pas dans baseq3.
run()
{
	step=$1
	shift
	"$Q3MAP2" -game quake3 -fs_basepath "$ROOT" -fs_game reach "$@" \
		"$work/$name.map" >"$work/$step.log" 2>&1 || {
		echo "map.sh: echec de la passe $step :" >&2
		tail -20 "$work/$step.log" >&2
		exit 1
	}
	# Une fuite (LEAK) veut dire que la salle n'est pas fermee.
	if grep -q LEAK "$work/$step.log"; then
		echo "map.sh: FUITE : la carte n'est pas fermee" >&2
		exit 1
	fi
	grep WARNING "$work/$step.log" || :
}

run bsp -meta
run vis -vis -fast
run light -light -fast -samplesize 16

cp "$work/$name.bsp" "$ROOT/reach/maps/$name.bsp"
echo "map.sh: reach/maps/$name.bsp"

#!/bin/sh
# test-niveau1.sh : joue le scénario du niveau 1 et juge le résultat.
#
# Porte fermée : le joueur s'arrête vers x = 104.
# Porte ouverte par le bouton : il traverse, jusque vers x = 240.

set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
out=$("$ROOT/tools/scenario.sh" orion_niveau1 niveau1)
echo "$out"

# Les deux premières coordonnées x des lignes « (x y z) : a ».
xs=$(echo "$out" | sed -n 's/^(\([-0-9.]*\) .*/\1/p')
closed=$(echo "$xs" | sed -n 1p)
open=$(echo "$xs" | sed -n 2p)

[ -n "$closed" ] && [ -n "$open" ] || {
	echo "ECHEC : positions manquantes"
	exit 1
}

# sh n'a pas de flottants : awk compare.
awk -v c="$closed" -v o="$open" 'BEGIN {
	ok1 = (c > 95 && c < 110)
	ok2 = (o > 200)
	printf "porte fermee : x = %s  %s\n", c, ok1 ? "ok" : "ECHEC"
	printf "porte ouverte : x = %s  %s\n", o, ok2 ? "ok" : "ECHEC"
	exit !(ok1 && ok2)
}'

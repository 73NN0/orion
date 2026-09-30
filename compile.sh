#!/bin/sh
# compile.sh - construit tout ce qu'il faut pour lancer Orion.
#
#   ./compile.sh          configure si besoin, puis construit dans engine/build
#   ./compile.sh clean    efface engine/build d'abord
#
# Options CMake supplémentaires : CMAKE_ARGS="-DUSE_FREETYPE=ON" ./compile.sh
# Le journal complet est écrit dans engine/build/compile.log.

set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ENGINE=$ROOT/engine
BUILD=$ENGINE/build
LOG=$BUILD/compile.log

if [ ! -d "$ENGINE" ]; then
	echo "engine/ absent : lance d'abord scripts/bootstrap.sh" >&2
	exit 1
fi

if [ "${1-}" = clean ]; then
	rm -rf "$BUILD"
fi
mkdir -p "$BUILD"
: >"$LOG"

# 1. Configurer une seule fois ; CMake se reconfigure seul si un
#    CMakeLists.txt change.
if [ ! -f "$BUILD/CMakeCache.txt" ]; then
	echo "configuration..."
	# CMAKE_ARGS est volontairement non quoté : plusieurs options possibles.
	if ! cmake -S "$ENGINE" -B "$BUILD" -DCMAKE_BUILD_TYPE=Debug \
		-DBUILD_STANDALONE=ON ${CMAKE_ARGS-} >>"$LOG" 2>&1; then
		tail -n 30 "$LOG"
		echo "échec de la configuration (journal : $LOG)" >&2
		exit 1
	fi
fi

# 2. Repartir d'un reach/ propre : la copie de runtime_data ajoute et
#    remplace, mais ne supprime jamais un fichier retiré de reach/.
rm -rf "$BUILD/Debug/reach"

# 3. Le client, le serveur, le renderer, les trois QVM et les données.
#    Avec --target, CMake ne construit QUE les cibles nommées.
echo "compilation..."
if ! cmake --build "$BUILD" --parallel 4 --target \
	orion orionded renderer_opengl2 \
	uiqvm_reach cgameqvm_reach qagameqvm_reach \
	runtime_data >>"$LOG" 2>&1; then
	grep -n -E "error|Error" "$LOG" | head -n 20
	echo "échec de la compilation (journal : $LOG)" >&2
	exit 1
fi

echo "ok : $BUILD/Debug/orion"

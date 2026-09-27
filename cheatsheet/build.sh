#!/usr/bin/env bash
# Builds and runs one cheatsheet module example.
#
#   ./cheatsheet/build.sh 1          -> m1_rcore_window.c
#   ./cheatsheet/build.sh 9          -> m9_raudio.c
#   ./cheatsheet/build.sh m5_rshapes.c
#
# Requires the library to be built already (bin/Debug/libraylib.a).

set -euo pipefail

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(dirname "$DIR")"

if [ $# -lt 1 ]; then
    echo "usage: $0 <module-number | file.c>"
    echo "available modules:"
    ls "$DIR"/m*.c | xargs -n1 basename | sort -V | sed 's/^/  /'
    exit 1
fi

ARG="$1"
if [[ "$ARG" =~ ^[0-9]+$ ]]; then
    SOURCE=$(ls "$DIR"/m${ARG}_*.c 2>/dev/null | head -1 || true)
    [ -z "$SOURCE" ] && { echo "No such module: $ARG"; exit 1; }
else
    SOURCE="$DIR/$(basename "$ARG")"
fi

LIB="$ROOT/bin/Debug/libraylib.a"
[ -f "$LIB" ] || { echo "Missing $LIB. Run 'make' in $ROOT first."; exit 1; }

OUTPUT="$ROOT/bin/Debug/$(basename "${SOURCE%.c}")"

echo ">> building $(basename "$SOURCE")"
cc "$SOURCE" -o "$OUTPUT" \
    -std=c17 -Wall -Wextra -g \
    -I"$ROOT/build/external/raylib-master/src" \
    "$LIB" -lpthread -lm -ldl -lrt -lX11

echo ">> running $OUTPUT"
cd "$ROOT"          # so files written by module 4 land in the project root
exec "$OUTPUT"

#!/usr/bin/env bash
# Builds and runs one lesson of the course.
#
#   ./course2d/build.sh 1          -> lesson01_*.c
#   ./course2d/build.sh 5          -> lesson05_*.c
#   ./course2d/build.sh lesson04_movement.c
#
# Requires the library to be built already (bin/Debug/libraylib.a).
# If it is not, run 'make' at the root of the project first.

set -euo pipefail

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(dirname "$DIR")"

if [ $# -lt 1 ]; then
    echo "usage: $0 <lesson-number | file.c>"
    echo "available lessons:"
    ls "$DIR"/lesson*.c | xargs -n1 basename | sed 's/^/  /'
    exit 1
fi

ARG="$1"
if [[ "$ARG" =~ ^[0-9]+$ ]]; then
    SOURCE=$(ls "$DIR"/lesson$(printf "%02d" "$ARG")_*.c 2>/dev/null | head -1 || true)
    [ -z "$SOURCE" ] && { echo "No such lesson: $ARG"; exit 1; }
else
    SOURCE="$DIR/$(basename "$ARG")"
fi

LIB="$ROOT/bin/Debug/libraylib.a"
[ -f "$LIB" ] || { echo "Missing $LIB. Run 'make' in $ROOT first."; exit 1; }

OUTPUT="$ROOT/bin/Debug/$(basename "${SOURCE%.c}")"

echo ">> building $(basename "$SOURCE")"
cc "$SOURCE" -o "$OUTPUT" \
    -std=c17 -Wall -Wextra -Wno-unused-parameter -g \
    -I"$ROOT/build/external/raylib-master/src" \
    "$LIB" -lpthread -lm -ldl -lrt -lX11

echo ">> running $OUTPUT"
exec "$OUTPUT"

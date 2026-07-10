#!/usr/bin/env bash
# Generate compile_commands.json for CLion/clangd indexing of this devkitPro 3DS project.
# devkitARM silences the real compiler commands, so we synthesize the DB from the
# Makefile's flags instead. Re-run this after adding source files or new include dirs.
set -euo pipefail
cd "$(dirname "$0")"

DKP="${DEVKITPRO:-/opt/devkitpro}"
CXX="$DKP/devkitARM/bin/arm-none-eabi-g++"
ROOT="$PWD"

# Mirrors the Makefile: ARCH + CXXFLAGS + include dirs (portlibs, libctru, generated build/).
FLAGS="-march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft \
-g -Wall -O2 -mword-relocations -ffunction-sections -D__3DS__ \
-fno-rtti -fno-exceptions -std=gnu++11 \
-I$ROOT/include -I$ROOT/source \
-I$DKP/portlibs/3ds/include -I$DKP/libctru/include -I$ROOT/build"

{
  echo "["
  first=1
  while IFS= read -r -d '' f; do
    [ $first -eq 1 ] && first=0 || echo ","
    printf '  {\n    "directory": "%s",\n    "file": "%s",\n    "command": "%s %s -c %s"\n  }' \
      "$ROOT" "$f" "$CXX" "$FLAGS" "$f"
  done < <(find "$ROOT/source" -type f \( -name '*.cpp' -o -name '*.c' \) -print0 | sort -z)
  echo
  echo "]"
} > compile_commands.json

echo "Wrote compile_commands.json ($(grep -c '"file"' compile_commands.json) entries)."

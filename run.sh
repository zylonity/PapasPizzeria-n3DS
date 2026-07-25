#!/usr/bin/env bash
# Build the app and launch it in Azahar.
set -euo pipefail

cd "$(dirname "$0")"

# CLion / IDE runs don't source your login shell, so make devkitPro visible here.
if [ -f /etc/profile.d/devkit-env.sh ]; then
    # shellcheck disable=SC1091
    . /etc/profile.d/devkit-env.sh
fi
export DEVKITPRO="${DEVKITPRO:-/opt/devkitpro}"
export DEVKITARM="${DEVKITARM:-$DEVKITPRO/devkitARM}"
export PATH="$DEVKITPRO/tools/bin:$DEVKITARM/bin:$PATH"

# shellcheck disable=SC1091
. "$(dirname "$0")/azahar-config.sh"

TARGET="$(basename "$PWD")"        # -> PapasPizzeria-n3DS (matches Makefile's $(notdir $(CURDIR)))
THREEDSX="$PWD/$TARGET.3dsx"

echo ">> Building $TARGET ..."
# Optimisation level is baked into the .o files; switching debug->release needs a clean.
if [ "$(cat .buildmode 2>/dev/null)" != "release" ]; then
    make clean >/dev/null 2>&1 || true
fi
make -j"$(nproc)"
echo release > .buildmode

echo ">> Disabling GDB stub for a normal run ..."
stop_azahar
set_azahar_gdbstub false

echo ">> Launching $TARGET.3dsx in AzaharPlus ..."
exec "$AZAHAR_BIN" "$THREEDSX"

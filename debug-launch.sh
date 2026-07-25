#!/usr/bin/env bash
# Build for debugging, start Azahar's GDB stub, and wait for CLion to attach.
set -euo pipefail
cd "$(dirname "$0")"

if [ -f /etc/profile.d/devkit-env.sh ]; then
    # shellcheck disable=SC1091
    . /etc/profile.d/devkit-env.sh
fi
export DEVKITPRO="${DEVKITPRO:-/opt/devkitpro}"
export DEVKITARM="${DEVKITARM:-$DEVKITPRO/devkitARM}"
export PATH="$DEVKITPRO/tools/bin:$DEVKITARM/bin:$PATH"

# shellcheck disable=SC1091
. "$(dirname "$0")/azahar-config.sh"

TARGET="$(basename "$PWD")"
THREEDSX="$PWD/$TARGET.3dsx"
PORT=4003

# Optimisation level is baked into the .o files, so switching release<->debug needs a clean.
if [ "$(cat .buildmode 2>/dev/null)" != "debug" ]; then
    echo ">> Switching to debug build (-O0), cleaning ..."
    make clean >/dev/null 2>&1 || true
fi
echo ">> Building $TARGET (DEBUG=1, -O0) ..."
make -j"$(nproc)" DEBUG=1
echo debug > .buildmode

# Close any running emulator (frees the port) and enable the stub for this launch.
stop_azahar
set_azahar_gdbstub true "$PORT"

echo ">> Launching AzaharPlus in background (halts on GDB stub :$PORT) ..."
# Give Azahar its own session so CLion won't reap it with this task.
setsid "$AZAHAR_BIN" "$THREEDSX" </dev/null >/tmp/azahar-debug.log 2>&1 &
disown 2>/dev/null || true

echo ">> Waiting for GDB stub on :$PORT ..."
for _ in $(seq 1 60); do
    if ss -tln 2>/dev/null | grep -q ":$PORT "; then
        echo ">> Stub is up — CLion will now attach gdb."
        exit 0
    fi
    sleep 0.5
done
echo "!! Timed out waiting for the stub (30s). See /tmp/azahar-debug.log" >&2
exit 1

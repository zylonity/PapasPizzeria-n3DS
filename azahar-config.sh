#!/usr/bin/env bash
# Shared helper for the launch scripts: toggle Azahar's GDB stub in its config,
# and cleanly stop any running instance so config edits aren't clobbered on exit.
# Sourced by run.sh (stub off) and debug-launch.sh (stub on).

# AzaharPlus (AppImage, native — not the flatpak). Override AZAHAR_BIN if you move it.
AZAHAR_BIN="${AZAHAR_BIN:-$HOME/Applications/AzaharPlus.AppImage}"
AZAHAR_CFG="$HOME/.config/azaharplus-emu/qt-config.ini"
# Match on "AzaharPlus" (not ".AppImage") so we also catch the re-exec'd binary
# running from the AppImage's /tmp/.mount_* path, not just the launcher.
AZAHAR_MATCH='AzaharPlus'

# Kill any running Azahar and wait until it's actually gone (it rewrites config on
# exit, and holds the GDB port). Escalates to SIGKILL if it doesn't stop cleanly.
stop_azahar() {
    pkill -if "$AZAHAR_MATCH" 2>/dev/null || true
    for _ in $(seq 1 25); do
        pgrep -if "$AZAHAR_MATCH" >/dev/null 2>&1 || return 0
        sleep 0.2
    done
    pkill -9 -if "$AZAHAR_MATCH" 2>/dev/null || true
    sleep 0.3
}

# set_azahar_gdbstub <true|false> [port] — flip the stub in the (already-existing) config.
set_azahar_gdbstub() {
    local val="$1" port="${2:-4003}"
    if [ ! -f "$AZAHAR_CFG" ]; then
        echo "!! Azahar config not found ($AZAHAR_CFG) — launch Azahar once so it's created." >&2
        return 0
    fi
    if grep -q '^use_gdbstub=' "$AZAHAR_CFG"; then
        sed -i "s/^use_gdbstub=.*/use_gdbstub=$val/" "$AZAHAR_CFG"
    elif grep -q '^\[Debugging\]' "$AZAHAR_CFG"; then
        sed -i "/^\[Debugging\]/a use_gdbstub=$val" "$AZAHAR_CFG"
    else
        printf '\n[Debugging]\nuse_gdbstub=%s\n' "$val" >> "$AZAHAR_CFG"
    fi
    if grep -q '^gdbstub_port=' "$AZAHAR_CFG"; then
        sed -i "s/^gdbstub_port=.*/gdbstub_port=$port/" "$AZAHAR_CFG"
    elif grep -q '^\[Debugging\]' "$AZAHAR_CFG"; then
        sed -i "/^\[Debugging\]/a gdbstub_port=$port" "$AZAHAR_CFG"
    fi
    # Azahar's qt-config stores a `key\default=true|false` companion; when it's
    # `true` the emulator ignores the stored value and resets it to the built-in
    # default on load (so the stub would never turn on). Force these to `false`
    # so our explicit values are honoured. (No-op if the companion line is absent.)
    sed -i 's/^use_gdbstub\\default=.*/use_gdbstub\\default=false/' "$AZAHAR_CFG"
    sed -i 's/^gdbstub_port\\default=.*/gdbstub_port\\default=false/' "$AZAHAR_CFG"
}

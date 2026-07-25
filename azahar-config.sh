#!/usr/bin/env bash
# Shared Azahar config helper for run.sh and debug-launch.sh.

# AzaharPlus (AppImage, native — not the flatpak). Override AZAHAR_BIN if you move it.
AZAHAR_BIN="${AZAHAR_BIN:-$HOME/Applications/AzaharPlus.AppImage}"
AZAHAR_CFG="$HOME/.config/azaharplus-emu/qt-config.ini"
# Match AzaharPlus so the AppImage's re-executed binary is included.
AZAHAR_MATCH='AzaharPlus'

# Stop Azahar fully before editing the config or reusing its GDB port.
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
    # Disable companion defaults so Azahar respects the values above.
    sed -i 's/^use_gdbstub\\default=.*/use_gdbstub\\default=false/' "$AZAHAR_CFG"
    sed -i 's/^gdbstub_port\\default=.*/gdbstub_port\\default=false/' "$AZAHAR_CFG"
}

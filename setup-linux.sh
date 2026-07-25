#!/usr/bin/env bash
# Install the Linux toolchain, video library, and Azahar; safe to rerun.
set -euo pipefail

DKP="${DEVKITPRO:-/opt/devkitpro}"
PORT3DS="$DKP/portlibs/3ds"

echo "==> [1/3] Installing devkitPro portlibs (needs sudo for pacman) ..."
# citro2d/citro3d/ctru are already installed; these are the audio/video deps the Makefile links.
sudo pacman -S --needed --noconfirm \
    3ds-sdl 3ds-sdl_mixer \
    3ds-libtheora 3ds-libogg 3ds-libvorbisidec \
    3ds-libmad 3ds-mikmod

echo "==> [2/3] Installing 3ds-LibTheoraPlayer from your GitHub release ..."
if [ -f "$PORT3DS/lib/lib3ds-LibTheoraPlayer.a" ]; then
    echo "    Already installed, skipping."
else
    REPO="zylonity/3ds-LibTheoraPlayer"
    tmp="$(mktemp -d)"
    trap 'rm -rf "$tmp"' EXIT
    echo "    Fetching latest release asset list ..."
    # Grab every asset from the latest release.
    mapfile -t urls < <(curl -fsSL "https://api.github.com/repos/$REPO/releases/latest" \
        | grep -oE '"browser_download_url":\s*"[^"]+"' | cut -d'"' -f4)
    if [ "${#urls[@]}" -eq 0 ]; then
        echo "    !! No release assets found. Download the release manually and copy its"
        echo "       lib3ds-LibTheoraPlayer.a -> $PORT3DS/lib/ and headers -> $PORT3DS/include/"
        exit 1
    fi
    for u in "${urls[@]}"; do
        echo "    Downloading $(basename "$u")"
        curl -fsSL -o "$tmp/$(basename "$u")" "$u"
    done
    # Unpack any archives in place.
    ( cd "$tmp"
      for f in *; do
        case "$f" in
          *.tar.gz|*.tgz) tar xzf "$f" ;;
          *.tar.bz2)      tar xjf "$f" ;;
          *.tar.xz)       tar xJf "$f" ;;
          *.zip)          unzip -oq "$f" ;;
        esac
      done )
    # Copy either a portlibs layout or the archive's loose libraries and headers.
    if [ -d "$tmp/lib" ] || [ -d "$tmp/include" ]; then
        [ -d "$tmp/lib" ]     && sudo cp -rv "$tmp/lib/."     "$PORT3DS/lib/"
        [ -d "$tmp/include" ] && sudo cp -rv "$tmp/include/." "$PORT3DS/include/"
    else
        find "$tmp" -name '*.a' -exec sudo cp -v {} "$PORT3DS/lib/" \;
        find "$tmp" \( -name '*.h' -o -name '*.hpp' \) -exec sudo cp -v {} "$PORT3DS/include/" \;
    fi
    if [ ! -f "$PORT3DS/lib/lib3ds-LibTheoraPlayer.a" ]; then
        echo "    !! Expected lib3ds-LibTheoraPlayer.a not found after install."
        echo "       Check the release contents and copy the .a into $PORT3DS/lib/ manually."
        exit 1
    fi
    echo "    Installed theoraplayer into portlibs."
fi

echo "==> [3/3] Installing the AzaharPlus emulator (AppImage) ..."
# Use the native AzaharPlus AppImage; the Flatpak's sandboxed audio is silent.
APPIMG="$HOME/Applications/AzaharPlus.AppImage"
if [ -x "$APPIMG" ]; then
    echo "    AzaharPlus already installed at $APPIMG, skipping."
else
    mkdir -p "$HOME/Applications"
    echo "    Resolving latest AzaharPlus Linux AppImage ..."
    url="$(curl -fsSL https://api.github.com/repos/AzaharPlus/AzaharPlus/releases/latest \
        | grep -oE '"browser_download_url":\s*"[^"]+linux\.AppImage"' | cut -d'"' -f4 | head -1)"
    if [ -z "$url" ]; then
        echo "    !! Could not find a linux .AppImage asset. Download it manually from"
        echo "       https://github.com/AzaharPlus/AzaharPlus/releases -> $APPIMG"
        exit 1
    fi
    echo "    Downloading $(basename "$url") ..."
    curl -fL -o "$APPIMG" "$url"
    chmod +x "$APPIMG"
    echo "    Installed AzaharPlus to $APPIMG."
fi

echo
echo "All set. Build & launch with:  ./run.sh"
echo
echo "Two manual steps AzaharPlus needs the first time (see linux-dev-setup notes):"
echo "  1. RENDERER: set Graphics API to *Vulkan* (Emulation > Configure > Graphics)."
echo "     OpenGL fails on Wayland ('Could not create EGL surface') -> black screen."
echo "  2. HOMEBREW AUDIO: copy your dumped DSP firmware to the emulated SD card at"
echo "       ~/.local/share/azaharplus-emu/sdmc/3ds/dspfirm.cdc"
echo "     Without it, every .3dsx is silent (ndsp can't load the DSP component);"
echo "     commercial titles have audio regardless. dspfirm.cdc can't be redistributed."

#!/data/data/com.termux/files/usr/bin/bash
# ---------------------------------------------------------------------------
# termux_build.sh
#
# One-file build script for Termux (Android). Run this AFTER downloading
# FreeTemplateBuilder-v3-with-autodeco.zip to your phone's Downloads folder.
#
# What it does:
#   1. Gives Termux access to your phone storage (one-time prompt).
#   2. Finds the mod zip in your Downloads folder automatically.
#   3. Installs build tools (git, cmake, clang, unzip) if missing.
#   4. Unzips the project into ~/ftb-build.
#   5. Downloads the Geode SDK (first run only, then reuses it).
#   6. Builds the mod.
#   7. Copies the finished .geode file back into your Downloads folder.
#
# Usage (inside Termux):
#   1. Save/move this file into Termux's home, e.g.:
#        termux-setup-storage
#        cp /sdcard/Download/termux_build.sh ~/
#   2. chmod +x termux_build.sh
#   3. ./termux_build.sh
# ---------------------------------------------------------------------------
set -euo pipefail

ZIP_NAME_HINT="FreeTemplateBuilder"
PROJECT_DIR="$HOME/ftb-build"
SDK_DIR="$HOME/.geode-sdk"

echo "== termux_build.sh: Free Template Builder =="

# ---------------------------------------------------------------------------
# 1. Storage access (so we can see /sdcard/Download)
# ---------------------------------------------------------------------------
if [ ! -d "$HOME/storage/downloads" ]; then
    echo "-> Requesting storage access (accept the Android permission popup)..."
    termux-setup-storage
    sleep 2
fi

DOWNLOADS_DIR="$HOME/storage/downloads"
if [ ! -d "$DOWNLOADS_DIR" ]; then
    echo "ERROR: Can't see your Downloads folder yet."
    echo "Run 'termux-setup-storage', accept the permission popup, then re-run this script."
    exit 1
fi

# ---------------------------------------------------------------------------
# 2. Find the mod zip in Downloads
# ---------------------------------------------------------------------------
mapfile -t ALL_MATCHES < <(find -L "$DOWNLOADS_DIR" -maxdepth 1 -iname "*${ZIP_NAME_HINT}*.zip" | sort)

if [ "${#ALL_MATCHES[@]}" -eq 0 ]; then
    echo "ERROR: Couldn't find a file matching '*${ZIP_NAME_HINT}*.zip' in:"
    echo "  $DOWNLOADS_DIR"
    echo "Make sure you downloaded the mod zip there first, then re-run this script."
    exit 1
fi

# Prefer the "-with-autodeco" build explicitly, since that's the complete
# version. Falls back to whatever single match exists otherwise.
ZIP_PATH=""
for f in "${ALL_MATCHES[@]}"; do
    if [[ "$f" == *"with-autodeco"* ]]; then
        ZIP_PATH="$f"
        break
    fi
done

if [ -z "$ZIP_PATH" ]; then
    if [ "${#ALL_MATCHES[@]}" -gt 1 ]; then
        echo "WARNING: Found multiple matching zips and none is the '-with-autodeco' version:"
        printf '  %s\n' "${ALL_MATCHES[@]}"
        echo "Using the most recently modified one:"
        ZIP_PATH=$(ls -t "${ALL_MATCHES[@]}" | head -n1)
    else
        ZIP_PATH="${ALL_MATCHES[0]}"
    fi
fi

if [ "${#ALL_MATCHES[@]}" -gt 1 ] && [[ "$ZIP_PATH" == *"with-autodeco"* ]]; then
    echo "NOTE: Multiple matching zips found; picked the '-with-autodeco' one on purpose:"
    printf '  %s\n' "${ALL_MATCHES[@]}"
fi

echo "-> Using mod zip: $ZIP_PATH"

# ---------------------------------------------------------------------------
# 3. Install build tools
# ---------------------------------------------------------------------------
echo "-> Checking/installing packages (git, cmake, clang, unzip)..."
pkg update -y
pkg install -y git cmake clang unzip

# ---------------------------------------------------------------------------
# 4. Unzip project
# ---------------------------------------------------------------------------
rm -rf "$PROJECT_DIR"
mkdir -p "$PROJECT_DIR"
unzip -q "$ZIP_PATH" -d "$PROJECT_DIR"

# The zip contains a top-level "FreeTemplateBuilder-v3" folder; find it.
SRC_DIR=$(find -L "$PROJECT_DIR" -maxdepth 1 -mindepth 1 -type d | head -n1)
if [ -z "$SRC_DIR" ] || [ ! -f "$SRC_DIR/CMakeLists.txt" ]; then
    echo "ERROR: Couldn't find CMakeLists.txt after unzipping. Zip layout may have changed."
    exit 1
fi
echo "-> Project unzipped to: $SRC_DIR"

# ---------------------------------------------------------------------------
# 5. Geode SDK
# ---------------------------------------------------------------------------
if [ ! -d "$SDK_DIR/.git" ]; then
    echo "-> Cloning Geode SDK (first time only, this may take a while)..."
    git clone --recursive https://github.com/geode-sdk/geode.git "$SDK_DIR"
else
    echo "-> Updating existing Geode SDK..."
    git -C "$SDK_DIR" fetch origin
    # A previous run may have been interrupted mid-clone, leaving the repo
    # on a detached HEAD (not on any branch). Force it onto main before
    # pulling so this never fails with "not currently on a branch".
    if ! git -C "$SDK_DIR" symbolic-ref -q HEAD >/dev/null; then
        echo "-> Repo was on a detached HEAD (interrupted previous run); fixing..."
        git -C "$SDK_DIR" checkout -B main origin/main
    fi
    git -C "$SDK_DIR" pull
    git -C "$SDK_DIR" submodule update --init --recursive
fi
export GEODE_SDK="$SDK_DIR"

# ---------------------------------------------------------------------------
# 6. Build
# ---------------------------------------------------------------------------
BUILD_DIR="$SRC_DIR/build"
mkdir -p "$BUILD_DIR"

echo "-> Configuring with CMake..."
cmake -B "$BUILD_DIR" -S "$SRC_DIR" -DCMAKE_BUILD_TYPE=Release

echo "-> Building..."
cmake --build "$BUILD_DIR" --config Release -j"$(nproc)"

# ---------------------------------------------------------------------------
# 7. Copy result back to Downloads
# ---------------------------------------------------------------------------
OUT_FILE=$(find -L "$BUILD_DIR" -maxdepth 3 -name "*.geode" | head -n1 || true)

if [ -n "$OUT_FILE" ]; then
    cp "$OUT_FILE" "$DOWNLOADS_DIR/"
    echo ""
    echo "== BUILD SUCCEEDED =="
    echo "Saved to: $DOWNLOADS_DIR/$(basename "$OUT_FILE")"
    echo "Copy/move it into your Geode mods folder, then open Geometry Dash."
else
    echo ""
    echo "== Build finished but no .geode file was produced. =="
    echo "Scroll up for the actual CMake/compiler error and send it back for a fix."
    exit 1
fi

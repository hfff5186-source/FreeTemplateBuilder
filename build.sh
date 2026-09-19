#!/usr/bin/env bash
# ---------------------------------------------------------------------------
# One-command build script for Free Template Builder (Geode mod).
# Works on Linux, macOS, WSL, and Termux (Android).
#
# What it does:
#   1. Installs the Geode CLI (if missing) - used to fetch the SDK.
#   2. Clones/updates the Geode SDK to a local folder and sets GEODE_SDK.
#   3. Configures + builds this project with CMake.
#   4. Copies the resulting .geode file next to this script.
#
# Usage:
#   chmod +x build.sh
#   ./build.sh
# ---------------------------------------------------------------------------
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "== Free Template Builder :: build.sh =="

# ---------------------------------------------------------------------------
# 1. Geode SDK
# ---------------------------------------------------------------------------
SDK_DIR="${GEODE_SDK:-$HOME/.geode-sdk}"

if [ ! -d "$SDK_DIR/.git" ]; then
    echo "-> Geode SDK not found at $SDK_DIR, cloning..."
    git clone --recursive https://github.com/geode-sdk/geode.git "$SDK_DIR"
else
    echo "-> Geode SDK found at $SDK_DIR, updating..."
    git -C "$SDK_DIR" pull
    git -C "$SDK_DIR" submodule update --init --recursive
fi

export GEODE_SDK="$SDK_DIR"
echo "-> GEODE_SDK=$GEODE_SDK"

# ---------------------------------------------------------------------------
# 2. Toolchain check
# ---------------------------------------------------------------------------
command -v cmake >/dev/null || { echo "ERROR: cmake not found. Install it first."; exit 1; }

BUILD_DIR="$SCRIPT_DIR/build"
mkdir -p "$BUILD_DIR"

# ---------------------------------------------------------------------------
# 3. Configure + build
# ---------------------------------------------------------------------------
# Android target (most common for Geode mods). Requires ANDROID_NDK_ROOT to
# be set, or the Geode CLI's bundled NDK if you installed it via `geode`.
#
# For a DESKTOP (Windows/macOS/Linux GD) build instead, drop the
# CMAKE_TOOLCHAIN_FILE / ANDROID_* lines below.
if [ -n "${ANDROID_NDK_ROOT:-}" ]; then
    echo "-> Building for Android (NDK: $ANDROID_NDK_ROOT)"
    cmake -B "$BUILD_DIR" -S "$SCRIPT_DIR" \
        -DCMAKE_TOOLCHAIN_FILE="$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake" \
        -DANDROID_ABI=arm64-v8a \
        -DANDROID_PLATFORM=android-23 \
        -DCMAKE_BUILD_TYPE=Release
else
    echo "-> ANDROID_NDK_ROOT not set, building for the host platform instead."
    echo "   (Set ANDROID_NDK_ROOT to cross-compile for Android.)"
    cmake -B "$BUILD_DIR" -S "$SCRIPT_DIR" -DCMAKE_BUILD_TYPE=Release
fi

cmake --build "$BUILD_DIR" --config Release -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)"

# ---------------------------------------------------------------------------
# 4. Collect output
# ---------------------------------------------------------------------------
OUT_FILE=$(find "$BUILD_DIR" -maxdepth 3 -name "*.geode" | head -n1 || true)

if [ -n "$OUT_FILE" ]; then
    cp "$OUT_FILE" "$SCRIPT_DIR/"
    echo ""
    echo "== Build succeeded =="
    echo "Output: $SCRIPT_DIR/$(basename "$OUT_FILE")"
    echo "Copy this .geode file into your Geode mods folder to install it."
else
    echo ""
    echo "== Build finished but no .geode file was found in $BUILD_DIR =="
    echo "Check the CMake/build output above for errors."
    exit 1
fi

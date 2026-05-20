#!/usr/bin/env bash

set -euo pipefail

general() {
    # Detect Platform
    OS=$(uname)
    PROFILE=linux

    if [ "$OS" = "Darwin" ]; then
        PROFILE=macos
    fi
}

linux() {
    echo "Configuring Linux..."

    if ! command -v apt >/dev/null 2>&1; then
        echo "apt is required for this Linux setup. Install dependencies manually for this distro."
        exit 1
    fi

    # Install dependencies
    DEPENDENCIES=(
        libgl1-mesa-dev
        mesa-common-dev
        libglfw3-dev
        libglew-dev
        libglm-dev
        cmake
        ninja-build
        clang-format
    )

    sudo apt update && sudo apt install -y --no-install-recommends "${DEPENDENCIES[@]}"
}

macos() {
    echo "Configuring macOS..."

    if ! command -v brew >/dev/null 2>&1; then
        echo "Homebrew is required. Install from https://brew.sh and re-run."
        exit 1
    fi

    # Install dependencies
    DEPENDENCIES=(
        glew
        glfw
        glm
        cmake
        clang-format
        ninja
    )
    
    brew install "${DEPENDENCIES[@]}"
}

# --- Main execution flow ---
general

if [ "$PROFILE" = "linux" ]; then
    linux
elif [ "$PROFILE" = "macos" ]; then
    macos
fi

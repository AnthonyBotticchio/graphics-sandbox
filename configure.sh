#/bin/bash

set -u

general() {
    # Detect Platform
    OS=$(uname)
    PROFILE=linux

    if [ "$OS" == "Darwin" ]; then
        PROFILE=macos
    fi
}

linux() {
    # TODO
    exit
}

macos() {
    echo "Configuring macOS..."

    if ! command -v brew >/dev/null 2>&1; then
        echo "Homebrew is required. Install from https://brew.sh and re-run."
        exit 1
    fi

    # Install dependencies
    DEPENDENCIES=(glew glfw glm cmake)
    brew install "${DEPENDENCIES[@]}"
}

# --- Main execution flow ---
general

if [ "$PROFILE" = "linux" ]; then
    linux
elif [ "$PROFILE" = "macos" ]; then
    macos
fi

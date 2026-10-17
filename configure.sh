#!/usr/bin/env bash

set -euo pipefail

general() {
    # Detect Platform
    OS=$(uname)
    case "$OS" in
        Linux) PROFILE=linux ;;
        Darwin) PROFILE=macos ;;
        *) echo "Unsupported platform: $OS. Note: use CMake and vcpkg on Windows." >&2; exit 1 ;;
    esac
    cd "$(dirname "${BASH_SOURCE[0]}")"
}

linux() {
    echo "Configuring Linux..."

    if ! command -v apt >/dev/null 2>&1; then
        echo "apt is required for this Linux setup. Install dependencies manually for this distro."
        exit 1
    fi

    # Install dependencies
    DEPENDENCIES=(
        git
        pkg-config
        libgl-dev
        libegl-dev
        libglfw3-dev
        libglm-dev
        libvulkan-dev
        glslc
        cmake
        ninja-build
        clang-format
        build-essential
        libyaml-cpp-dev
    )

    sudo apt update

    # Ubuntu 26.04 packages the shader Slang compiler; older distros may not.
    if apt-cache show slang-compiler >/dev/null 2>&1; then
        DEPENDENCIES+=(slang-compiler)
    fi

    sudo apt install -y --no-install-recommends "${DEPENDENCIES[@]}"

    if ! pkg-config --atleast-version=3.4 glfw3; then
        echo "GLFW 3.4+ with Wayland support is required. Upgrade libglfw3-dev." >&2
        return 1
    fi
}

macos() {
    echo "Configuring macOS..."

    if ! command -v brew >/dev/null 2>&1; then
        echo "Homebrew is required. Install from https://brew.sh and re-run."
        exit 1
    fi

    if ! xcode-select -p >/dev/null 2>&1; then
        echo "Install Apple's Command Line Tools with xcode-select --install, then re-run." >&2
        return 1
    fi

    # Install dependencies
    DEPENDENCIES=(
        glfw
        glm
        shaderc
        cmake
        clang-format
        ninja
        yaml-cpp
        coreutils
    )
    
    brew install "${DEPENDENCIES[@]}"

    local sdk_setup="" candidate answer sdk_version download_dir installer
    # VULKAN_SDK normally points to the SDK's macOS subdirectory.
    if [ -n "${VULKAN_SDK:-}" ]; then
        for candidate in "$VULKAN_SDK/../setup-env.sh" "$VULKAN_SDK/setup-env.sh"; do
            if [ -f "$candidate" ]; then
                sdk_setup="$candidate"
                break
            fi
        done
    fi
    if [ -z "$sdk_setup" ]; then
        # Prefer the newest installed version in LunarG's default location.
        sdk_setup=$(find "$HOME/VulkanSDK" -mindepth 2 -maxdepth 2 -name setup-env.sh -type f 2>/dev/null |
            sort -V | tail -n 1) || true
    fi

    if [ -z "$sdk_setup" ]; then
        if ! read -r -p "VULKAN SDK NOT FOUND. DO YOU WANT TO INSTALL THE LATEST VULKAN SDK? [Y/N] " answer; then
            answer=n
        fi
        case "$answer" in
            [Yy]|[Yy][Ee][Ss]) ;;
            *) echo "Skipping Vulkan SDK installation."; return ;;
        esac

        sdk_version=$(curl -fsSL https://vulkan.lunarg.com/sdk/latest/mac.txt)
        if [[ ! "$sdk_version" =~ ^[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+$ ]]; then
            echo "Could not determine the latest Vulkan SDK version." >&2
            return 1
        fi
        download_dir=$(mktemp -d)
        if ! curl -fL "https://sdk.lunarg.com/sdk/download/$sdk_version/mac/vulkan_sdk.zip" \
                -o "$download_dir/vulkan_sdk.zip" ||
                ! ditto -x -k "$download_dir/vulkan_sdk.zip" "$download_dir"; then
            rm -rf "$download_dir"
            return 1
        fi
        installer=$(find "$download_dir" -type f -path '*/Contents/MacOS/*' -iname 'vulkansdk-macos-*' -print -quit)
        if [ -z "$installer" ] || ! "$installer" --root "$HOME/VulkanSDK/$sdk_version" \
                --accept-licenses --default-answer --confirm-command install copy_only=1; then
            echo "Vulkan SDK installation failed." >&2
            rm -rf "$download_dir"
            return 1
        fi
        rm -rf "$download_dir"
        sdk_setup="$HOME/VulkanSDK/$sdk_version/setup-env.sh"
    fi

    printf 'To enable Vulkan in your current terminal, run: source %q\n' "$sdk_setup"
}

check_tools() {
    local cmake_version
    cmake_version=$(cmake --version)
    cmake_version=${cmake_version%%$'\n'*}
    if [[ ! "$cmake_version" =~ ([0-9]+)\.([0-9]+) ]] ||
        (( BASH_REMATCH[1] < 3 || (BASH_REMATCH[1] == 3 && BASH_REMATCH[2] < 28) )); then
        echo "CMake 3.28+ is required; found $cmake_version." >&2
        return 1
    fi
    if ! command -v slangc >/dev/null 2>&1; then
        echo "Shader builds need slangc. Enable the Vulkan SDK or install Slang from https://shader-slang.org/tools/."
    fi
}

# --- Main execution flow ---
general

if [ "$PROFILE" = "linux" ]; then
    linux
elif [ "$PROFILE" = "macos" ]; then
    macos
fi

git submodule update --init --recursive
check_tools

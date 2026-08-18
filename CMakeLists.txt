cmake_minimum_required(VERSION 3.28)

# --- Toolchain ---

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
    file(TO_CMAKE_PATH "$ENV{VCPKG_ROOT}" VCPKG_ROOT_PATH)
    set(VCPKG_TOOLCHAIN_FILE "${VCPKG_ROOT_PATH}/scripts/buildsystems/vcpkg.cmake")
    set(VCPKG_INSTALL_OPTIONS "--no-print-usage") # Silences vcpkg usage output

    if(WIN32)
        set(VCPKG_TARGET_TRIPLET "x64-windows")
    endif()

    if(EXISTS "${VCPKG_TOOLCHAIN_FILE}")
        message(STATUS "Found vcpkg toolchain: ${VCPKG_TOOLCHAIN_FILE}")
        set(CMAKE_TOOLCHAIN_FILE "${VCPKG_TOOLCHAIN_FILE}" CACHE STRING "CMake toolchain file")
    else()
        message(WARNING "VCPKG_ROOT is set, but vcpkg toolchain was not found: ${VCPKG_TOOLCHAIN_FILE}")
    endif()
endif()

# --- Project ---

project(graphics-sandbox LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install)

# Modify CMake modules path
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/external/cmake-scripts)

# --- Platforms ---

include(GNUInstallDirs)

if(UNIX)
    set(BUILD_SHARED_LIBS ON)

    set(CMAKE_INSTALL_BINDIR bin)
    set(CMAKE_INSTALL_LIBDIR lib)

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # Target
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # Shared libs
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) # Static libs

    set(CMAKE_INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")

    if(APPLE)
        # Ensure homebrew packages are discoverable
        list(APPEND ${CMAKE_PREFIX_PATH} "/opt/homebrew")
        set(CMAKE_INSTALL_RPATH "@loader_path/../${CMAKE_INSTALL_LIBDIR}")
    endif()
elseif(WIN32)
    set(BUILD_SHARED_LIBS OFF)

    set(CMAKE_INSTALL_BINDIR bin)
    set(CMAKE_INSTALL_LIBDIR bin)

    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

    if(MSVC)
        add_compile_options(
            /W3
            /WX
        )
    endif()
endif()

# --- Formatting ---

# include(formatting)

# file(GLOB_RECURSE CL_FILES
#     *.[hc]
#     *.[hc]pp
#     *.mm        # OBJCXX
# )

# list(FILTER CL_FILES EXCLUDE REGEX "${CMAKE_CURRENT_SOURCE_DIR}/build")
# list(FILTER CL_FILES EXCLUDE REGEX "${CMAKE_CURRENT_SOURCE_DIR}/install")
# list(FILTER CL_FILES EXCLUDE REGEX "${CMAKE_CURRENT_SOURCE_DIR}/external")
# clang_format(format ${CL_FILES})

# file(GLOB_RECURSE CM_FILES CMakeLists.txt *.cmake)
# list(FILTER CM_FILES EXCLUDE REGEX ${CMAKE_CURRENT_SOURCE_DIR}/build)
# list(FILTER CM_FILES EXCLUDE REGEX ${CMAKE_CURRENT_SOURCE_DIR}/external)
# cmake_format(cmake-format ${CM_FILES})

# --- Subdirs ---

add_subdirectory(opengl)
add_subdirectory(external SYSTEM)

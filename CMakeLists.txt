cmake_minimum_required(VERSION 3.28)

# --- Project ---

project(graphics-sandbox LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install)

# Modify CMake modules path
# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/external/cmake-scripts)

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

# --- Platforms ---

if(APPLE)
    # Ensure homebrew packages are discoverable
    list(APPEND ${CMAKE_PREFIX_PATH} "/opt/homebrew")
endif()

# --- Subdirs ---

add_subdirectory(opengl)
add_subdirectory(logger)

# --- Installs ---

install(DIRECTORY textures/ DESTINATION textures)

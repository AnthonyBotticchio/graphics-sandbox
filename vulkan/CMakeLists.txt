# --- Vulkan Dependencies ---

if(APPLE)
    find_package(GLFW3 REQUIRED)
    find_package(glm REQUIRED)
    find_package(Vulkan REQUIRED)
elseif(UNIX AND NOT APPLE)
    find_package(Vulkan REQUIRED)
    find_package(glfw3 REQUIRED)
    find_package(glm REQUIRED)
elseif(WIN32)
    # Use of keyword CONFIG for vcpkg
    find_package(Vulkan REQUIRED)
    find_package(glm CONFIG REQUIRED)
    find_package(glfw3 CONFIG REQUIRED)
endif()

# --- Resources ---

# For ebajec/engine
add_compile_options(
    -DRESOURCE_PATH=${CMAKE_CURRENT_SOURCE_DIR}/resource
)

# --- Subdirs ---

add_subdirectory(particle)
# add_subdirectory(utils)
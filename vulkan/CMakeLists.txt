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

# --- Subdirs ---

add_subdirectory(particle)
# add_subdirectory(utils)
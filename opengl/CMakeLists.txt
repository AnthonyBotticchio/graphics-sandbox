# --- OGL Dependencies ---

if(APPLE)
    find_package(OpenGL 4.1 REQUIRED)
    find_package(GLFW3 REQUIRED)
    find_package(glm REQUIRED)
elseif(UNIX AND NOT APPLE)
    find_package(OpenGL REQUIRED)
    find_package(glfw3 REQUIRED)
    find_package(glm REQUIRED)
elseif(WIN32)
    # Use of keyword CONFIG for vcpkg
    find_package(OpenGL REQUIRED)
    find_package(glm CONFIG REQUIRED)
    find_package(glfw3 CONFIG REQUIRED)
    find_package(GLEW CONFIG REQUIRED)
endif()

find_package(GLEW REQUIRED)

# --- Subdirs ---

add_subdirectory(sample)
add_subdirectory(utilities SYSTEM)
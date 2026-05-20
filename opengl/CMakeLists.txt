# --- OGL Dependencies ---

if(APPLE)
    find_package(OpenGL 4.1 REQUIRED)
    find_package(GLFW3 REQUIRED)
elseif(UNIX AND NOT APPLE)
    find_package(OpenGL REQUIRED)
    find_package(glfw3 REQUIRED)
elseif(WIN32)
    message(FATAL_ERROR "OGL dependencies not defined yet!")
endif()

find_package(glm REQUIRED)
find_package(GLEW REQUIRED)

# --- Subdirs ---

add_subdirectory(sample)
add_subdirectory(utilities SYSTEM)
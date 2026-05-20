# --- OGL Dependencies ---

if(APPLE)
    find_package(OpenGL 4.1 REQUIRED)
    find_package(GLFW3 REQUIRED)
endif()

if(UNIX AND NOT APPLE)
    find_package(OpenGL REQUIRED)
    find_package(glfw3 REQUIRED)
endif()

find_package(glm REQUIRED)
find_package(GLEW REQUIRED)

# --- Subdirs ---

add_subdirectory(sample)
add_subdirectory(utilities SYSTEM)
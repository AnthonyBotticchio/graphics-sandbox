# --- Dependencies ---

if(APPLE OR IOS)
    find_package(GLFW3 REQUIRED)
    find_package(glm REQUIRED)
    find_package(GLEW REQUIRED)
endif()

if(UNIX AND NOT APPLE)
    message(WARNING "TODO: Linux")
endif()

find_package(OpenGL 4.1 REQUIRED)

# --- Subdirs ---

add_subdirectory(sample)
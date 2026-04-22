# --- Dependencies ---

if(APPLE OR IOS)
    find_package(OpenGL 4.1 REQUIRED)
endif()

if(UNIX AND NOT APPLE)
    message(WARNING "TODO: Linux")
    find_package(OpenGL REQUIRED)
endif()

find_package(GLFW3 REQUIRED)
find_package(glm REQUIRED)
find_package(GLEW REQUIRED)

# --- Subdirs ---

add_subdirectory(sample)
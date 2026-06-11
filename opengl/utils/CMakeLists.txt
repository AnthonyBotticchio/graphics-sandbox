# --- TARGET ---

set(TARGET utils)

add_library(${TARGET})

# Release mode optimizations
if(UNIX)
    target_compile_options(${TARGET} PRIVATE 
        $<$<CONFIG:Release>:-O3 -ffast-math -march=native> # -march=native targets local CPU optimizations
    )
endif()

# --- Dependencies ---

target_link_libraries(${TARGET} PUBLIC
    OpenGL::GL
    glfw
    glm::glm
    GLEW::GLEW
    logger
)

# --- Sources ---

add_subdirectory(include)
add_subdirectory(src)

# --- Install ---

install(TARGETS ${TARGET}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

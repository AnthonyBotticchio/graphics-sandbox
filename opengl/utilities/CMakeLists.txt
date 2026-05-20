# --- TARGET ---

set(TARGET utilities)

add_library(${TARGET})

# Release mode optimizations
target_compile_options(${TARGET} PRIVATE 
    $<$<CONFIG:Release>:-O3 -ffast-math -march=native> # -march=native targets local CPU optimizations
)

target_compile_definitions(${TARGET} PRIVATE
    TIME_BLOCK_QUIET
)

# --- Dependencies ---

target_link_libraries(${TARGET} PUBLIC
    OpenGL::GL
    glfw
    glm::glm
    GLEW::GLEW
    logger
)

# --- Sources ---

target_include_directories(${TARGET} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

file(GLOB_RECURSE CXX_SOURCES   "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE CXX_HEADERS   "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp")

target_sources(${TARGET} PRIVATE
    ${CXX_SOURCES}
    ${CXX_HEADERS}
)
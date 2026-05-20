# --- Executable ---

set(TARGET sample)

add_executable(${TARGET})

set_target_properties(${TARGET} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin # Target
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib # Shared libs
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib # Static libs
)

# Release mode optimizations
target_compile_options(${TARGET} PRIVATE 
    $<$<CONFIG:Release>:-O3 -ffast-math -march=native> # -march=native targets local CPU optimizations
)

# --- Dependencies ---

target_link_libraries(${TARGET} PRIVATE
    OpenGL::GL
    glfw
    glm::glm
    GLEW::GLEW
    logger
    utilities
)

# --- Sources ---

file(GLOB_RECURSE CXX_SOURCES           "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE CXX_HEADERS   "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

target_sources(${TARGET} PRIVATE
    ${CXX_SOURCES}
    ${CXX_HEADERS}
)

# --- Install ---

set(SHADER_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/shader)
set(TEXTURE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/textures)

install(TARGETS     ${TARGET} RUNTIME DESTINATION bin)
install(DIRECTORY   ${SHADER_DIR}     DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY   ${TEXTURE_DIR}    DESTINATION ${CMAKE_INSTALL_PREFIX})

# --- Commands ---

add_custom_command(TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${SHADER_DIR}
            ${CMAKE_BINARY_DIR}/shader
    COMMENT "Copying shaders..."
)

add_custom_command(TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${TEXTURE_DIR}
            ${CMAKE_BINARY_DIR}/textures
    COMMENT "Copying textures..."
)

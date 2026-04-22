# --- Executable ---

set(TARGET sample)

add_executable(${TARGET})

set_target_properties(${TARGET} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin # Target
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib # Shared libs
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib # Static libs
)

# --- Dependencies ---

target_link_libraries(${TARGET} PRIVATE
    OpenGL::GL
    glfw
    glm::glm
    GLEW::GLEW
    logger
)

# --- Sources ---

file(GLOB_RECURSE CXX_SOURCES           "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE CXX_PRIVATE_HEADERS   "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

target_sources(${TARGET} PRIVATE
    ${CXX_SOURCES}
    ${CXX_PRIVATE_HEADERS}
)

# --- Install ---

set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shader)
set(TEXTURE_DIR ${CMAKE_SOURCE_DIR}/textures)

install(TARGETS ${TARGET} RUNTIME DESTINATION bin)
install(DIRECTORY ${SHADER_DIR}/ DESTINATION shader)

# --- Commands ---

add_custom_target(copy_shaders ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${SHADER_DIR}/
            ${CMAKE_BINARY_DIR}/shader
    COMMENT "Copying shaders..."
)

add_custom_target(copy_textures ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${TEXTURE_DIR}/
            ${CMAKE_BINARY_DIR}/textures
    COMMENT "Copying textures..."
)

add_dependencies(${TARGET} copy_shaders copy_textures)

# --- Executable ---

set(TARGET mtl-sample)

add_executable(${TARGET})

# --- Dependencies ---

target_link_libraries(${TARGET} PRIVATE
    metal-cpp
    glfw
    logger
)

# --- Sources ---

add_subdirectory(src)

# --- Resources ---

set(SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shader)
set(TEXTURE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/textures)

add_custom_target(${TARGET}-resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_newer
            ${SHADER_DIR} 
            ${CMAKE_BINARY_DIR}/shader
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_newer
            ${TEXTURE_DIR} 
            ${CMAKE_BINARY_DIR}/textures
    COMMENT "Copying MTL shaders and textures..."
)

add_dependencies(${TARGET} ${TARGET}-resources)

# --- Install ---

install(TARGETS     ${TARGET} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY   ${SHADER_DIR}     DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY   ${TEXTURE_DIR}    DESTINATION ${CMAKE_INSTALL_PREFIX})
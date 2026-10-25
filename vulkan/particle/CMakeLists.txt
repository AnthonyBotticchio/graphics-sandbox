# --- Executable ---

set(TARGET particle)

add_executable(${TARGET})

# --- Dependencies ---

if(APPLE)
    target_compile_options(${TARGET} PUBLIC -fPIC)
    target_compile_definitions(${TARGET} PUBLIC 
        VK_ENABLE_BETA_EXTENSIONS=1
        VK_USE_PLATFORM_METAL_EXT=1
        VK_RENDERER_USE_GLFW=1
    )
endif()

target_link_libraries(${TARGET} PRIVATE
    logger
    ev2-engine
    ev2-editor
)

# --- Sources ---

add_subdirectory(include)
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
    COMMENT "Copying VK shaders and textures..."
)

add_dependencies(${TARGET} ${TARGET}-resources)

# --- Install ---

install(TARGETS     ${TARGET} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY   ${SHADER_DIR}     DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY   ${TEXTURE_DIR}    DESTINATION ${CMAKE_INSTALL_PREFIX})
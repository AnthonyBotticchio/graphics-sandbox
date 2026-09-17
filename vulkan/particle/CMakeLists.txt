# --- Executable ---

set(TARGET particle)

add_executable(${TARGET})

# Release mode optimizations
if(UNIX)
    target_compile_options(${TARGET} PRIVATE 
        $<$<CONFIG:Release>:-O3 -ffast-math -march=native> # -march=native targets local CPU optimizations
    )
endif()

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
    engine
)

# --- Sources ---

# add_subdirectory(precompiled)
add_subdirectory(include)
add_subdirectory(src)

# --- Install ---

set(SHADER_DIR  ${CMAKE_CURRENT_SOURCE_DIR}/shader)
set(TEXTURE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/textures)

install(TARGETS     ${TARGET} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
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

# --- TARGET ---

set(TARGET opengl-engine)

add_library(${TARGET})

# --- Dependencies ---

if(APPLE)
    find_library(APPKIT_FRAMEWORK AppKit REQUIRED)
    find_library(COREGRAPHICS_FRAMEWORK CoreGraphics REQUIRED)
    find_library(QUARTZCORE_FRAMEWORK QuartzCore REQUIRED)
    target_link_libraries(${TARGET} PRIVATE ${APPKIT_FRAMEWORK} ${COREGRAPHICS_FRAMEWORK} ${QUARTZCORE_FRAMEWORK})
endif()

target_link_libraries(${TARGET} PUBLIC
    OpenGL::GL
    glfw
    glm::glm
    glad-4.1
    logger
)

# --- Sources ---

add_subdirectory(include)
add_subdirectory(src)

# --- Install ---

install(TARGETS ${TARGET}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Install include files
install(DIRECTORY include/engine DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

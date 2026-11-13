# --- Target ---

set(TARGET metal-engine)

add_library(${TARGET})

# --- Dependencies ---

target_link_libraries(${TARGET} PUBLIC
    metal-cpp
    SDL3::SDL3
    glm::glm
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

install(DIRECTORY include/engine DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

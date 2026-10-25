# --- Library Setup ---

set(TARGET glad-4.1)
add_library(${TARGET} src/glad.c)
set_target_properties(${TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(${TARGET} SYSTEM PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(${TARGET} PUBLIC GLFW_INCLUDE_NONE)
target_link_libraries(${TARGET} PRIVATE ${CMAKE_DL_LIBS})

# --- Install ---

install(TARGETS ${TARGET}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

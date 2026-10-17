# --- Library Setup ---

add_library(graphics-glad STATIC src/glad.c)
set_target_properties(graphics-glad PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(graphics-glad SYSTEM PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(graphics-glad PUBLIC GLFW_INCLUDE_NONE)
target_link_libraries(graphics-glad PRIVATE ${CMAKE_DL_LIBS})

# --- Sources ---

target_include_directories(${TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

file(GLOB_RECURSE CXX_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE CXX_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

list(FILTER CXX_SOURCES EXCLUDE REGEX "/platform/frame_pacer_glfw\\.cpp$")

if(APPLE)
    target_sources(${TARGET} PRIVATE platform/frame_pacer_macos.mm)
    set_source_files_properties(platform/frame_pacer_macos.mm PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
else()
    target_sources(${TARGET} PRIVATE platform/frame_pacer_glfw.cpp)
endif()

target_sources(${TARGET} PRIVATE
    ${CXX_SOURCES}
    ${CXX_HEADERS}
)

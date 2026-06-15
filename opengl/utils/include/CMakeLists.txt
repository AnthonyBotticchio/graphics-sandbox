# --- Sources ---

target_include_directories(${TARGET} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

file(GLOB_RECURSE CXX_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")

target_sources(${TARGET} PRIVATE
    ${CXX_HEADERS}
)
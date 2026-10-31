# --- Sources ---

file(GLOB CXX_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

target_sources(${TARGET} PRIVATE ${CXX_HEADERS})

# --- Subdirs ---

add_subdirectory(platform)

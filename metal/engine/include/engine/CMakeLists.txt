# --- Sources ---

file(GLOB CXX_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

target_sources(${TARGET} PRIVATE ${CXX_HEADERS})

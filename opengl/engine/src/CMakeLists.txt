# --- Sources ---

file(GLOB CXX_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

target_sources(${TARGET} PRIVATE ${CXX_SOURCES})

# --- Subdirs ---

add_subdirectory(platform)

# --- TARGET ---

set(TARGET logger)

add_library(${TARGET})

# --- Sources ---

target_include_directories(${TARGET} 
    SYSTEM PUBLIC # Use generator expressions to describe separate usage requirements based on usage location.
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>    # for build time
        $<INSTALL_INTERFACE:include>                              # for install time
)

file(GLOB_RECURSE C_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
file(GLOB_RECURSE C_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

target_sources(${TARGET} PRIVATE 
    ${C_SOURCES} 
    ${C_HEADERS}
)

# --- Install ---

install(TARGETS ${TARGET}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# --- TARGET ---

add_library(logger)

# --- Sources ---

target_include_directories(logger 
    SYSTEM PUBLIC # Use generator expressions to describe separate usage requirements based on usage location.
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>    # for build time
        $<INSTALL_INTERFACE:include>                              # for install time
)

file(GLOB_RECURSE C_SOURCES             "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
file(GLOB_RECURSE C_HEADERS     "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

target_sources(logger PRIVATE 
    ${C_SOURCES} 
    ${C_HEADERS}
)
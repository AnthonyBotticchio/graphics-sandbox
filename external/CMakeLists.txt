# --- External Subdirs ---

add_subdirectory(logger)
add_subdirectory(glad)

if(NOT WIN32)
    add_subdirectory(engine)
endif()

# --- Metal C++ ---

if(APPLE)
    add_library(metal-cpp INTERFACE)
    target_include_directories(metal-cpp SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/metal-cpp)
    # target_compile_features(metal-cpp INTERFACE cxx_std_17)

    find_library(METAL_FRAMEWORK Metal REQUIRED)
    find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
    find_library(QUARTZCORE_FRAMEWORK QuartzCore REQUIRED)
    target_link_libraries(metal-cpp INTERFACE
        ${METAL_FRAMEWORK}
        ${FOUNDATION_FRAMEWORK}
        ${QUARTZCORE_FRAMEWORK}
    )
endif()

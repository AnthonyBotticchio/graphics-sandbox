# --- Sources ---

if(APPLE)
    target_sources(${TARGET} PRIVATE
        frame_pacer_macos.mm
        performance_monitor.mm
    )

    # -fobj-arc is needed for automatic reference counting. Used in the display link objects in Impl
    set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/frame_pacer_macos.mm"
        TARGET_DIRECTORY ${TARGET}
        PROPERTIES COMPILE_OPTIONS "-fobjc-arc"
    )
else()
    target_sources(${TARGET} PRIVATE
        frame_pacer_glfw.cpp
        performance_monitor.cpp
    )
endif()


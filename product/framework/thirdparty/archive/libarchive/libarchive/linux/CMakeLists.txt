# =============================================================================
# libarchive - Linux Platform (Prebuilt)
# =============================================================================

set(LA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/x64")

if(NOT EXISTS "${LA_ROOT}/include/archive.h")
    message(FATAL_ERROR "[libarchive] Prebuilt not found at ${LA_ROOT}. Run GitHub Action 'build-libarchive-all-platforms.yml' first.")
endif()

find_library(LA_LIBRARY
    NAMES archive libarchive
    PATHS "${LA_ROOT}/lib"
    NO_DEFAULT_PATH
)

if(NOT LA_LIBRARY)
    message(FATAL_ERROR "[libarchive] Dynamic library not found in ${LA_ROOT}/lib")
endif()

file(GLOB LA_SOLIBS "${LA_ROOT}/lib/*.so*")

message(STATUS "[libarchive] Linux x64")
message(STATUS "[libarchive] Dynamic Library: ${LA_LIBRARY}")
message(STATUS "[libarchive] Shared Objects: ${LA_SOLIBS}")

if(LA_SOLIBS)
    add_custom_target(copy_libarchive)
    set_target_properties(copy_libarchive PROPERTIES FOLDER framework/thirdparty/archive)

    add_custom_command(TARGET copy_libarchive POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${LA_SOLIBS} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/"
        COMMAND_EXPAND_LISTS
        COMMENT "[libarchive] Copying shared objects to output directory"
    )
endif()

add_library(libarchive_prebuilt INTERFACE)
target_include_directories(libarchive_prebuilt INTERFACE "${LA_ROOT}/include")
target_link_libraries(libarchive_prebuilt INTERFACE "${LA_LIBRARY}")

if(TARGET copy_libarchive)
    add_dependencies(libarchive_prebuilt copy_libarchive)
endif()

set_target_properties(libarchive_prebuilt PROPERTIES
    INTERFACE_LINK_OPTIONS "-Wl,-rpath,$ORIGIN"
)

add_library(archive::multi ALIAS libarchive_prebuilt)

if(LA_SOLIBS)
    install(FILES ${LA_SOLIBS} DESTINATION bin)
endif()

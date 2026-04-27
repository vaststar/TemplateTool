# =============================================================================
# libarchive - macOS Platform (Prebuilt)
# =============================================================================

if(CMAKE_OSX_ARCHITECTURES MATCHES "arm64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "arm64")
    set(LA_ARCH_DIR "arm")
else()
    set(LA_ARCH_DIR "intel")
endif()

set(LA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${LA_ARCH_DIR}")

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

file(GLOB LA_DYLIBS "${LA_ROOT}/lib/*.dylib")

message(STATUS "[libarchive] macOS ${LA_ARCH_DIR}")
message(STATUS "[libarchive] Dynamic Library: ${LA_LIBRARY}")
message(STATUS "[libarchive] Dylibs: ${LA_DYLIBS}")

if(LA_DYLIBS)
    add_custom_target(copy_libarchive)
    set_target_properties(copy_libarchive PROPERTIES FOLDER framework/thirdparty/archive)

    add_custom_command(TARGET copy_libarchive POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${LA_DYLIBS} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/"
        COMMAND_EXPAND_LISTS
        COMMENT "[libarchive] Copying dylibs to output directory"
    )
endif()

add_library(libarchive_prebuilt INTERFACE)
target_include_directories(libarchive_prebuilt INTERFACE "${LA_ROOT}/include")
target_link_libraries(libarchive_prebuilt INTERFACE "${LA_LIBRARY}")

if(TARGET copy_libarchive)
    add_dependencies(libarchive_prebuilt copy_libarchive)
endif()

add_library(archive::multi ALIAS libarchive_prebuilt)

if(LA_DYLIBS)
    install(FILES ${LA_DYLIBS} DESTINATION bin)
endif()

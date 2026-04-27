# =============================================================================
# libarchive - Android Platform (Prebuilt)
# =============================================================================

if(NOT ANDROID_ABI)
    message(FATAL_ERROR "[libarchive] ANDROID_ABI not defined")
endif()

set(LA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/${ANDROID_ABI}")

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

message(STATUS "[libarchive] Android ${ANDROID_ABI}")
message(STATUS "[libarchive] Dynamic Library: ${LA_LIBRARY}")

add_library(libarchive_prebuilt INTERFACE)
target_include_directories(libarchive_prebuilt INTERFACE "${LA_ROOT}/include")
target_link_libraries(libarchive_prebuilt INTERFACE "${LA_LIBRARY}")

add_library(archive::multi ALIAS libarchive_prebuilt)
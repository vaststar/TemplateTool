# ==========================================
# macOS Post-Install Script
# ==========================================
# This script handles dylib deployment for macOS builds
# - Copies third-party dylibs to bin directory
# - Moves all dynamic libraries to app bundle Frameworks directory

set(INSTALL_PREFIX "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}")
set(APP_NAME "mainEntry")
set(BIN_DIR "${INSTALL_PREFIX}/bin")
set(APP_BUNDLE "${BIN_DIR}/${APP_NAME}.app")
set(DEPLOY_DIR "${APP_BUNDLE}/Contents/Frameworks")
set(THIRDPARTY_DIR "${TEMPLATE_TOOL_SOURCE_DIR}/product/framework/thirdparty")

# Detect architecture - use multiple methods since CMAKE_* variables may not be available at install time
set(MACOS_ARCH "")

# Method 1: Check CMAKE_OSX_ARCHITECTURES first (if set during configure)
if(CMAKE_OSX_ARCHITECTURES MATCHES "arm64")
    set(MACOS_ARCH "macosx_arm64_release")
elseif(CMAKE_OSX_ARCHITECTURES MATCHES "x86_64")
    set(MACOS_ARCH "macosx_x86_64_release")
endif()

# Method 2: Check CMAKE_SYSTEM_PROCESSOR
if(NOT MACOS_ARCH)
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "arm64")
        set(MACOS_ARCH "macosx_arm64_release")
    elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
        set(MACOS_ARCH "macosx_x86_64_release")
    endif()
endif()

# Method 3: Use uname command at install time (most reliable)
if(NOT MACOS_ARCH)
    execute_process(
        COMMAND uname -m
        OUTPUT_VARIABLE HOST_ARCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(HOST_ARCH STREQUAL "arm64")
        set(MACOS_ARCH "macosx_arm64_release")
    else()
        set(MACOS_ARCH "macosx_x86_64_release")
    endif()
endif()

message(STATUS "")
message(STATUS "========================================")
message(STATUS " macOS Dylib Deployment")
message(STATUS "========================================")
message(STATUS "Install Prefix  : ${INSTALL_PREFIX}")
message(STATUS "Source Dir      : ${TEMPLATE_TOOL_SOURCE_DIR}")
message(STATUS "ThirdParty Dir  : ${THIRDPARTY_DIR}")
message(STATUS "App Bundle      : ${APP_NAME}.app")
message(STATUS "Deploy Dir      : ${DEPLOY_DIR}")
message(STATUS "Architecture    : ${MACOS_ARCH}")
message(STATUS "========================================")
message(STATUS "")

# ==========================================
# Step 1: Create Frameworks directory
# ==========================================
message(STATUS "[1/3] Creating Frameworks directory...")

if(NOT EXISTS "${DEPLOY_DIR}")
    file(MAKE_DIRECTORY "${DEPLOY_DIR}")
    message(STATUS "  ✓ Created Frameworks directory")
else()
    message(STATUS "  ✓ Frameworks directory exists")
endif()

message(STATUS "")

# ==========================================
# Step 2: Copy Third-Party Dylibs
# ==========================================
message(STATUS "[2/3] Copying third-party dylibs...")

set(THIRDPARTY_DYLIBS
    "${THIRDPARTY_DIR}/openssl/macosx/${MACOS_ARCH}/lib/libssl.dylib"
    "${THIRDPARTY_DIR}/openssl/macosx/${MACOS_ARCH}/lib/libcrypto.dylib"
    # Add more thirdparty dylibs here as needed
)

set(COPIED_COUNT 0)
set(SKIPPED_COUNT 0)
set(FAILED_COUNT 0)

foreach(dylib ${THIRDPARTY_DYLIBS})
    if(NOT EXISTS "${dylib}")
        get_filename_component(dylib_name "${dylib}" NAME)
        message(STATUS "  ⊘ Skipped: ${dylib_name} (not found at ${dylib})")
        math(EXPR SKIPPED_COUNT "${SKIPPED_COUNT} + 1")
        continue()
    endif()
    
    get_filename_component(dylib_name "${dylib}" NAME)
    set(dest "${BIN_DIR}/${dylib_name}")
    
    # Remove existing file
    if(EXISTS "${dest}")
        file(REMOVE "${dest}")
    endif()
    
    # Copy dylib
    file(COPY "${dylib}" DESTINATION "${BIN_DIR}")
    
    if(EXISTS "${dest}")
        message(STATUS "  ✓ Copied: ${dylib_name}")
        math(EXPR COPIED_COUNT "${COPIED_COUNT} + 1")
    else()
        message(STATUS "  ✗ Failed: ${dylib_name}")
        math(EXPR FAILED_COUNT "${FAILED_COUNT} + 1")
    endif()
endforeach()

message(STATUS "")
message(STATUS "Third-party Dylibs Summary:")
message(STATUS "  Copied  : ${COPIED_COUNT}")
message(STATUS "  Skipped : ${SKIPPED_COUNT}")
message(STATUS "  Failed  : ${FAILED_COUNT}")
message(STATUS "")

# ==========================================
# Step 3: Move dynamic libraries to Frameworks
# ==========================================
message(STATUS "[3/3] Moving dynamic libraries to Frameworks...")

# Find all .dylib files in bin directory
file(GLOB DYLIB_FILES "${BIN_DIR}/*.dylib")

set(MOVED_COUNT 0)
set(MOVE_SKIPPED_COUNT 0)
set(MOVE_FAILED_COUNT 0)

if(NOT DYLIB_FILES)
    message(STATUS "  ⊘ No .dylib files found in ${BIN_DIR}")
else()
    foreach(dylib_path ${DYLIB_FILES})
        if(NOT EXISTS "${dylib_path}")
            math(EXPR MOVE_SKIPPED_COUNT "${MOVE_SKIPPED_COUNT} + 1")
            continue()
        endif()

        get_filename_component(dylib_name "${dylib_path}" NAME)
        set(dest_path "${DEPLOY_DIR}/${dylib_name}")

        # Remove existing file to avoid RENAME failure
        if(EXISTS "${dest_path}")
            file(REMOVE "${dest_path}")
        endif()

        # Move dylib to Frameworks directory
        file(RENAME "${dylib_path}" "${dest_path}")
        
        if(EXISTS "${dest_path}")
            message(STATUS "  ✓ Moved: ${dylib_name}")
            math(EXPR MOVED_COUNT "${MOVED_COUNT} + 1")
        else()
            message(STATUS "  ✗ Failed: ${dylib_name}")
            math(EXPR MOVE_FAILED_COUNT "${MOVE_FAILED_COUNT} + 1")
        endif()
    endforeach()
endif()

message(STATUS "")
message(STATUS "Dynamic Libraries Summary:")
message(STATUS "  Moved   : ${MOVED_COUNT}")
message(STATUS "  Skipped : ${MOVE_SKIPPED_COUNT}")
message(STATUS "  Failed  : ${MOVE_FAILED_COUNT}")
message(STATUS "")

if(MOVE_FAILED_COUNT GREATER 0)
    message(WARNING "Some dylibs failed to move. Please check the log above.")
endif()

message(STATUS "========================================")
message(STATUS " Post-install completed!")
message(STATUS " Deploy location: ${DEPLOY_DIR}")
message(STATUS "========================================")

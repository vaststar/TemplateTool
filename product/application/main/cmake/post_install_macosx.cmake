# ==========================================
# macOS Post-Install Script
# ==========================================
# This script handles dylib deployment for macOS builds
# - Moves dynamic libraries to app bundle Frameworks directory
# - Verifies deployment results

set(INSTALL_PREFIX "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}")
set(APP_NAME "mainEntry")
set(BIN_DIR "${INSTALL_PREFIX}/bin")
set(APP_BUNDLE "${BIN_DIR}/${APP_NAME}.app")
set(DEPLOY_DIR "${APP_BUNDLE}/Contents/Frameworks")

message(STATUS "")
message(STATUS "========================================")
message(STATUS " macOS Dylib Deployment")
message(STATUS "========================================")
message(STATUS "Install Prefix : ${INSTALL_PREFIX}")
message(STATUS "Source Dir     : ${BIN_DIR}")
message(STATUS "App Bundle     : ${APP_NAME}.app")
message(STATUS "Deploy Dir     : ${DEPLOY_DIR}")
message(STATUS "========================================")
message(STATUS "")

# ==========================================
# Step 1: Create Frameworks directory
# ==========================================
message(STATUS "[1/2] Creating Frameworks directory...")

if(NOT EXISTS "${DEPLOY_DIR}")
    file(MAKE_DIRECTORY "${DEPLOY_DIR}")
    message(STATUS "  ✓ Created Frameworks directory")
else()
    message(STATUS "  ✓ Frameworks directory exists")
endif()

message(STATUS "")

# ==========================================
# Step 2: Moving dynamic libraries
# ==========================================
message(STATUS "[2/2] Moving dynamic libraries to Frameworks...")

# Find all .dylib files in bin directory
file(GLOB DYLIB_FILES "${BIN_DIR}/*.dylib")

set(MOVED_COUNT 0)
set(SKIPPED_COUNT 0)
set(FAILED_COUNT 0)

if(NOT DYLIB_FILES)
    message(STATUS "  ⊘ No .dylib files found in ${BIN_DIR}")
else()
    foreach(dylib_path ${DYLIB_FILES})
        if(NOT EXISTS "${dylib_path}")
            math(EXPR SKIPPED_COUNT "${SKIPPED_COUNT} + 1")
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
            math(EXPR FAILED_COUNT "${FAILED_COUNT} + 1")
        endif()
    endforeach()
endif()

message(STATUS "")
message(STATUS "Dynamic Libraries Summary:")
message(STATUS "  Moved   : ${MOVED_COUNT}")
message(STATUS "  Skipped : ${SKIPPED_COUNT}")
message(STATUS "  Failed  : ${FAILED_COUNT}")
message(STATUS "")

if(FAILED_COUNT GREATER 0)
    message(WARNING "Some dylibs failed to move. Please check the log above.")
endif()

message(STATUS "========================================")
message(STATUS " Post-install completed!")
message(STATUS " Deploy location: ${DEPLOY_DIR}")
message(STATUS "========================================")

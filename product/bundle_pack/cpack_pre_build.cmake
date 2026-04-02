# ==========================================
# CPack Pre-Build Script (all platforms)
# ==========================================
# Removes development-only files (headers, cmake exports) from the
# CPack staging directory before packaging.
# This avoids conflicts with system packages (e.g. sqlite3.h)
# and keeps the package lean — without touching any install() rules.
# ==========================================

# CPack populates CPACK_TEMPORARY_INSTALL_DIRECTORY before calling this script
set(STAGING_DIR "${CPACK_TEMPORARY_INSTALL_DIRECTORY}")

# Find the actual content root (may be nested under ALL_COMPONENTS_IN_ONE or directly)
if(EXISTS "${STAGING_DIR}/ALL_COMPONENTS_IN_ONE")
    set(CONTENT_ROOT "${STAGING_DIR}/ALL_COMPONENTS_IN_ONE")
else()
    set(CONTENT_ROOT "${STAGING_DIR}")
endif()

# Remove development files that should not be in runtime packages
set(DEV_DIRS
    "${CONTENT_ROOT}/include"
    "${CONTENT_ROOT}/cmake"
)

foreach(dev_dir ${DEV_DIRS})
    if(EXISTS "${dev_dir}")
        file(REMOVE_RECURSE "${dev_dir}")
        message(STATUS "[cpack_pre_build] Removed: ${dev_dir}")
    endif()
endforeach()

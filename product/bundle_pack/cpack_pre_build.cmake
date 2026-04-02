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

# Remove development files (headers, cmake exports) from all possible locations.
# The actual layout depends on CPACK_PACKAGING_INSTALL_PREFIX:
#   - Without prefix: STAGING_DIR/include/, STAGING_DIR/cmake/
#   - With /usr prefix: STAGING_DIR/usr/include/, STAGING_DIR/usr/cmake/
#   - With COMPONENTS: STAGING_DIR/ALL_COMPONENTS_IN_ONE/usr/include/, ...
file(GLOB_RECURSE _all_dirs LIST_DIRECTORIES true "${STAGING_DIR}/*")
foreach(_path ${_all_dirs})
    if(IS_DIRECTORY "${_path}")
        get_filename_component(_name "${_path}" NAME)
        # Only remove top-level include/ and cmake/ directories
        # (not nested ones inside bin/qml/ etc.)
        get_filename_component(_parent "${_path}" DIRECTORY)
        get_filename_component(_parent_name "${_parent}" NAME)
        if((_name STREQUAL "include" OR _name STREQUAL "cmake") AND NOT _parent_name STREQUAL "bin")
            if(EXISTS "${_path}")
                file(REMOVE_RECURSE "${_path}")
                message(STATUS "[cpack_pre_build] Removed: ${_path}")
            endif()
        endif()
    endif()
endforeach()

# ==========================================
# Relocate .desktop and icons to /usr/share/ for GNOME/KDE discovery.
# When the prefix is /opt/template-factory, these files end up under
# /opt/template-factory/share/ which desktop environments don't search.
# Move them to /usr/share/ inside the staging tree so the DEB installs
# them to the correct system-wide location.
# ==========================================
set(_opt_share "${STAGING_DIR}/opt/template-factory/share")
set(_usr_share "${STAGING_DIR}/usr/share")

# Relocate .desktop files
if(EXISTS "${_opt_share}/applications")
    file(MAKE_DIRECTORY "${_usr_share}/applications")
    file(GLOB _desktop_files "${_opt_share}/applications/*.desktop")
    foreach(_f ${_desktop_files})
        get_filename_component(_fname "${_f}" NAME)
        file(RENAME "${_f}" "${_usr_share}/applications/${_fname}")
        message(STATUS "[cpack_pre_build] Relocated: ${_fname} -> /usr/share/applications/")
    endforeach()
    file(REMOVE_RECURSE "${_opt_share}/applications")
endif()

# Relocate icons
if(EXISTS "${_opt_share}/icons")
    file(MAKE_DIRECTORY "${_usr_share}/icons")
    file(GLOB_RECURSE _icon_files "${_opt_share}/icons/*")
    foreach(_f ${_icon_files})
        if(NOT IS_DIRECTORY "${_f}")
            file(RELATIVE_PATH _rel "${_opt_share}/icons" "${_f}")
            get_filename_component(_rel_dir "${_rel}" DIRECTORY)
            file(MAKE_DIRECTORY "${_usr_share}/icons/${_rel_dir}")
            file(RENAME "${_f}" "${_usr_share}/icons/${_rel}")
            message(STATUS "[cpack_pre_build] Relocated: icons/${_rel} -> /usr/share/icons/${_rel}")
        endif()
    endforeach()
    file(REMOVE_RECURSE "${_opt_share}/icons")
endif()

# Remove empty share/ dir under /opt if nothing left
if(EXISTS "${_opt_share}")
    file(GLOB _remaining "${_opt_share}/*")
    if(NOT _remaining)
        file(REMOVE_RECURSE "${_opt_share}")
    endif()
endif()

# Create /usr/bin/mainEntry wrapper script.
# Simply exec's the real binary — no environment hacks needed.
# The bin/lib -> ../lib symlink handles library resolution via RUNPATH.
set(_wrapper_dir "${STAGING_DIR}/usr/bin")
file(MAKE_DIRECTORY "${_wrapper_dir}")
file(WRITE "${_wrapper_dir}/mainEntry"
"#!/bin/sh\n\
exec /opt/template-factory/bin/mainEntry \"$@\"\n"
)
execute_process(COMMAND chmod 755 "${_wrapper_dir}/mainEntry")
message(STATUS "[cpack_pre_build] Created: /usr/bin/mainEntry (wrapper script)")

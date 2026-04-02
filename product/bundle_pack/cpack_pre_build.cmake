# ==========================================
# CPack Pre-Build Script (all platforms)
# ==========================================
# Runs inside CPack's staging directory before the final archive.
# Responsibilities:
#   1. Strip development-only files (headers, cmake exports)
#   2. (Linux) Relocate .desktop / icons to /usr/share/
#   3. (Linux) Create /usr/bin wrapper script
# ==========================================

set(STAGING_DIR "${CPACK_TEMPORARY_INSTALL_DIRECTORY}")
set(_prefix "${CPACK_PACKAGING_INSTALL_PREFIX}")
set(_prefix_in_staging "${STAGING_DIR}${_prefix}")

# ==========================================
# 1. Strip development files (headers, cmake exports)
# ==========================================
# Explicitly target known dev directories rather than glob-recursing,
# which would follow the bin/lib -> ../lib symlink and accidentally
# act on lib/cmake/ through the symlink path.
foreach(_dev_dir "include" "cmake" "lib/cmake")
    set(_target "${_prefix_in_staging}/${_dev_dir}")
    if(EXISTS "${_target}")
        file(REMOVE_RECURSE "${_target}")
        message(STATUS "[cpack_pre_build] Removed: ${_target}")
    endif()
endforeach()

# ==========================================
# 2. Relocate .desktop and icons to /usr/share/
# ==========================================
# When the prefix is outside /usr (e.g. /opt/...), .desktop and icon
# files land under <prefix>/share/ which desktop environments don't
# search.  Move them to /usr/share/ so GNOME/KDE discover them.
# On Windows/macOS these paths don't exist, so the block is a no-op.
set(_pkg_share "${_prefix_in_staging}/share")
set(_usr_share "${STAGING_DIR}/usr/share")

if(EXISTS "${_pkg_share}/applications")
    file(MAKE_DIRECTORY "${_usr_share}/applications")
    file(GLOB _desktop_files "${_pkg_share}/applications/*.desktop")
    foreach(_f ${_desktop_files})
        get_filename_component(_fname "${_f}" NAME)
        file(RENAME "${_f}" "${_usr_share}/applications/${_fname}")
        message(STATUS "[cpack_pre_build] Relocated: ${_fname} -> /usr/share/applications/")
    endforeach()
    file(REMOVE_RECURSE "${_pkg_share}/applications")
endif()

if(EXISTS "${_pkg_share}/icons")
    file(MAKE_DIRECTORY "${_usr_share}/icons")
    file(GLOB_RECURSE _icon_files "${_pkg_share}/icons/*")
    foreach(_f ${_icon_files})
        if(NOT IS_DIRECTORY "${_f}")
            file(RELATIVE_PATH _rel "${_pkg_share}/icons" "${_f}")
            get_filename_component(_rel_dir "${_rel}" DIRECTORY)
            file(MAKE_DIRECTORY "${_usr_share}/icons/${_rel_dir}")
            file(RENAME "${_f}" "${_usr_share}/icons/${_rel}")
            message(STATUS "[cpack_pre_build] Relocated: icons/${_rel} -> /usr/share/icons/${_rel}")
        endif()
    endforeach()
    file(REMOVE_RECURSE "${_pkg_share}/icons")
endif()

# Remove empty share/ if nothing left
if(EXISTS "${_pkg_share}")
    file(GLOB _remaining "${_pkg_share}/*")
    if(NOT _remaining)
        file(REMOVE_RECURSE "${_pkg_share}")
    endif()
endif()

# ==========================================
# 3. Create /usr/bin wrapper script
# ==========================================
# A thin shell wrapper that exec's the real binary, allowing the app
# to live in a private prefix while remaining accessible via PATH.
# Library resolution is handled by RUNPATH + the bin/lib -> ../lib
# symlink created during install — no LD_LIBRARY_PATH needed.
# NOTE: _bin_name must match MAIN_ENTRY in application/main/CMakeLists.txt
set(_bin_name "mainEntry")
if(EXISTS "${_prefix_in_staging}/bin/${_bin_name}")
    set(_wrapper_dir "${STAGING_DIR}/usr/bin")
    file(MAKE_DIRECTORY "${_wrapper_dir}")
    file(WRITE "${_wrapper_dir}/${_bin_name}"
"#!/bin/sh\n\
exec ${_prefix}/bin/${_bin_name} \"$@\"\n"
    )
    execute_process(COMMAND chmod 755 "${_wrapper_dir}/${_bin_name}")
    message(STATUS "[cpack_pre_build] Created: /usr/bin/${_bin_name} (wrapper script)")
endif()

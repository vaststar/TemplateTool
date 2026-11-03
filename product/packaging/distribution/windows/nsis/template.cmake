# Keep CPack's file/uninstall logic, adapting only the directory page and the
# preinstall hook. Older CMake versions keep this template under Modules/.
set(_tt_nsis_source "${CMAKE_ROOT}/Modules/Internal/CPack/NSIS.template.in")
if(NOT EXISTS "${_tt_nsis_source}")
    set(_tt_nsis_source "${CMAKE_ROOT}/Modules/NSIS.template.in")
endif()
file(READ "${_tt_nsis_source}" _tt_nsis_template)
string(REPLACE "\r\n" "\n" _tt_nsis_template "${_tt_nsis_template}")

set(_tt_directory_page [=[  !insertmacro MUI_PAGE_DIRECTORY]=])
set(_tt_preinstall [=[  SetOutPath "$INSTDIR"
  @CPACK_NSIS_EXTRA_PREINSTALL_COMMANDS@]=])
foreach(_tt_anchor IN ITEMS "${_tt_directory_page}" "${_tt_preinstall}")
    string(REPLACE "${_tt_anchor}" "" _tt_without_anchor "${_tt_nsis_template}")
    string(LENGTH "${_tt_nsis_template}" _tt_template_length)
    string(LENGTH "${_tt_without_anchor}" _tt_without_length)
    string(LENGTH "${_tt_anchor}" _tt_anchor_length)
    math(EXPR _tt_removed_length "${_tt_template_length} - ${_tt_without_length}")
    if(NOT _tt_removed_length EQUAL _tt_anchor_length)
        message(FATAL_ERROR
            "Unsupported CPack NSIS template: ${_tt_nsis_source}. "
            "Review the TemplateTool installer hooks before packaging.")
    endif()
endforeach()

# A page-specific callback must be defined immediately before that page macro.
string(REPLACE "${_tt_directory_page}" [=[  !define MUI_PAGE_CUSTOMFUNCTION_PRE TTDirectoryPagePre
  !insertmacro MUI_PAGE_DIRECTORY]=] _tt_nsis_template "${_tt_nsis_template}")
# Re-check registration before CPack creates the selected directory.
string(REPLACE "${_tt_preinstall}" [=[  Call TTValidateBeforeInstall
  SetOutPath "$INSTDIR"
  @CPACK_NSIS_EXTRA_PREINSTALL_COMMANDS@]=] _tt_nsis_template "${_tt_nsis_template}")

set(_tt_nsis_module_dir "${CMAKE_CURRENT_BINARY_DIR}/cpack-modules")
file(MAKE_DIRECTORY "${_tt_nsis_module_dir}")
file(WRITE "${_tt_nsis_module_dir}/NSIS.template.in" "${_tt_nsis_template}")
if(NOT DEFINED CPACK_MODULE_PATH)
    set(CPACK_MODULE_PATH "${CMAKE_MODULE_PATH}")
endif()
list(PREPEND CPACK_MODULE_PATH "${_tt_nsis_module_dir}")

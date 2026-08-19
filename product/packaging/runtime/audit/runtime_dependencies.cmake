# Runtime dependency audit
#
# Runs after platform deployment and reports dependencies that are unresolved or
# still resolve outside the staged install tree. This first-stage audit never
# changes the install tree and never fails the install.

foreach(required_variable
        APP_NAME
        APPLICATION_FILE_NAME
        TEMPLATE_TOOL_BINARY_DIR)
    if(NOT DEFINED ${required_variable}
       OR "${${required_variable}}" STREQUAL "")
        message(WARNING
            "[runtime-audit] ${required_variable} is not set; skipping audit")
        return()
    endif()
endforeach()

set(audit_install_prefix "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}")
cmake_path(NORMAL_PATH audit_install_prefix)

if(APPLE)
    set(audit_platform "macos")
    set(audit_application_root
        "${audit_install_prefix}/bin/${APP_NAME}.app")
    set(audit_executable
        "${audit_application_root}/Contents/MacOS/${APPLICATION_FILE_NAME}")
    set(audit_library_roots
        "${audit_application_root}/Contents/Frameworks"
        "${audit_application_root}/Contents/PlugIns"
        "${audit_application_root}/Contents/Resources/qml"
    )
    set(audit_library_patterns "*.dylib" "*.so")
elseif(WIN32)
    set(audit_platform "windows")
    set(audit_application_root "${audit_install_prefix}/bin")
    set(audit_executable
        "${audit_application_root}/${APPLICATION_FILE_NAME}")
    set(audit_library_roots "${audit_application_root}")
    set(audit_library_patterns "*.dll")
else()
    set(audit_platform "linux")
    set(audit_application_root "${audit_install_prefix}")
    set(audit_executable
        "${audit_install_prefix}/bin/${APPLICATION_FILE_NAME}")
    set(audit_library_roots
        "${audit_install_prefix}/bin"
        "${audit_install_prefix}/lib"
    )
    set(audit_library_patterns "*.so" "*.so.*")
endif()

if(NOT EXISTS "${audit_executable}")
    message(WARNING
        "[runtime-audit] Application executable does not exist; "
        "skipping audit: ${audit_executable}")
    return()
endif()

# Include dynamically loaded plugins as roots. They are not necessarily visible
# from the executable's static dependency graph.
set(audit_library_candidates)
foreach(library_root IN LISTS audit_library_roots)
    if(NOT IS_DIRECTORY "${library_root}")
        continue()
    endif()

    foreach(library_pattern IN LISTS audit_library_patterns)
        file(GLOB_RECURSE matched_libraries
            LIST_DIRECTORIES FALSE
            "${library_root}/${library_pattern}")
        list(APPEND audit_library_candidates ${matched_libraries})
    endforeach()
endforeach()

# Versioned shared libraries often appear through multiple symlinks. Resolve
# them before passing roots to CMake to avoid scanning the same binary twice.
set(audit_libraries)
foreach(library_candidate IN LISTS audit_library_candidates)
    file(REAL_PATH "${library_candidate}" real_library)
    list(APPEND audit_libraries "${real_library}")
endforeach()
list(REMOVE_DUPLICATES audit_libraries)
list(REMOVE_ITEM audit_libraries "${audit_executable}")

set(runtime_dependency_arguments EXECUTABLES "${audit_executable}")
if(audit_libraries)
    list(APPEND runtime_dependency_arguments LIBRARIES ${audit_libraries})
endif()

# Windows has no rpath. The application directory is the intended lookup root
# for project and Qt DLLs deployed by windeployqt.
if(WIN32)
    list(APPEND runtime_dependency_arguments
        DIRECTORIES "${audit_application_root}")
endif()

file(GET_RUNTIME_DEPENDENCIES
    RESOLVED_DEPENDENCIES_VAR resolved_dependencies
    UNRESOLVED_DEPENDENCIES_VAR unresolved_dependencies
    CONFLICTING_DEPENDENCIES_PREFIX conflicting_dependencies
    ${runtime_dependency_arguments}
)

list(SORT resolved_dependencies)
list(SORT unresolved_dependencies)

function(runtime_dependency_is_system dependency output_variable)
    file(TO_CMAKE_PATH "${dependency}" normalized_dependency)

    if(APPLE)
        if(normalized_dependency MATCHES "^/System/Library/"
           OR normalized_dependency MATCHES "^/usr/lib/")
            set(is_system TRUE)
        else()
            set(is_system FALSE)
        endif()
    elseif(WIN32)
        set(system_root "$ENV{SystemRoot}")
        if(system_root STREQUAL "")
            set(system_root "$ENV{WINDIR}")
        endif()
        file(TO_CMAKE_PATH "${system_root}" system_root)
        string(TOLOWER "${system_root}" system_root)
        string(TOLOWER "${normalized_dependency}" normalized_dependency)

        if(system_root STREQUAL "")
            set(is_system FALSE)
        else()
            cmake_path(IS_PREFIX system_root "${normalized_dependency}"
                NORMALIZE is_system)
        endif()
    else()
        if(normalized_dependency MATCHES "^/lib/"
           OR normalized_dependency MATCHES "^/lib64/"
           OR normalized_dependency MATCHES "^/usr/lib/"
           OR normalized_dependency MATCHES "^/usr/lib64/")
            set(is_system TRUE)
        else()
            set(is_system FALSE)
        endif()
    endif()

    set(${output_variable} "${is_system}" PARENT_SCOPE)
endfunction()

set(packaged_dependencies)
set(system_dependencies)
set(external_dependencies)

foreach(dependency IN LISTS resolved_dependencies)
    file(REAL_PATH "${dependency}" normalized_dependency)
    cmake_path(IS_PREFIX audit_install_prefix "${normalized_dependency}"
        NORMALIZE dependency_is_packaged)

    if(dependency_is_packaged)
        list(APPEND packaged_dependencies "${normalized_dependency}")
    else()
        runtime_dependency_is_system(
            "${normalized_dependency}" dependency_is_system)
        if(dependency_is_system)
            list(APPEND system_dependencies "${normalized_dependency}")
        else()
            list(APPEND external_dependencies "${normalized_dependency}")
        endif()
    endif()
endforeach()

list(REMOVE_DUPLICATES packaged_dependencies)
list(REMOVE_DUPLICATES system_dependencies)
list(REMOVE_DUPLICATES external_dependencies)

set(conflicting_dependency_names
    ${conflicting_dependencies_FILENAMES})
list(SORT conflicting_dependency_names)

if(BUILD_CONFIG_NAME STREQUAL "")
    set(audit_config "default")
else()
    set(audit_config "${BUILD_CONFIG_NAME}")
endif()
string(MAKE_C_IDENTIFIER "${audit_config}" audit_config)
string(MAKE_C_IDENTIFIER "${APP_NAME}" audit_application_name)

set(audit_report_directory
    "${TEMPLATE_TOOL_BINARY_DIR}/packaging/runtime-audit/${audit_config}")
set(audit_report
    "${audit_report_directory}/${audit_application_name}-${audit_platform}.txt")
file(MAKE_DIRECTORY "${audit_report_directory}")

set(report_lines
    "Runtime dependency audit"
    "Application: ${audit_executable}"
    "Install tree: ${audit_install_prefix}"
    "Platform: ${audit_platform}"
    ""
)

macro(append_dependency_category title dependency_list)
    list(LENGTH ${dependency_list} dependency_count)
    list(APPEND report_lines "${title} (${dependency_count})")
    if(NOT "${${dependency_list}}" STREQUAL "")
        foreach(dependency_item IN LISTS ${dependency_list})
            list(APPEND report_lines "  ${dependency_item}")
        endforeach()
    else()
        list(APPEND report_lines "  <none>")
    endif()
    list(APPEND report_lines "")
endmacro()

append_dependency_category(
    "Packaged dependencies" packaged_dependencies)
append_dependency_category(
    "System dependencies" system_dependencies)
append_dependency_category(
    "External non-system dependencies" external_dependencies)
append_dependency_category(
    "Unresolved dependencies" unresolved_dependencies)

list(LENGTH conflicting_dependency_names conflicting_dependency_count)
list(APPEND report_lines
    "Conflicting dependency names (${conflicting_dependency_count})")
if(conflicting_dependency_names)
    foreach(conflicting_name IN LISTS conflicting_dependency_names)
        set(conflicting_paths_variable
            "conflicting_dependencies_${conflicting_name}")
        list(APPEND report_lines "  ${conflicting_name}")
        foreach(conflicting_path IN LISTS ${conflicting_paths_variable})
            list(APPEND report_lines "    ${conflicting_path}")
        endforeach()
    endforeach()
else()
    list(APPEND report_lines "  <none>")
endif()

string(JOIN "\n" report_content ${report_lines})
file(WRITE "${audit_report}" "${report_content}\n")

list(LENGTH packaged_dependencies packaged_dependency_count)
list(LENGTH system_dependencies system_dependency_count)
list(LENGTH external_dependencies external_dependency_count)
list(LENGTH unresolved_dependencies unresolved_dependency_count)

message(STATUS "")
message(STATUS "========================================")
message(STATUS " Runtime Dependency Audit (report only)")
message(STATUS "========================================")
message(STATUS "Packaged    : ${packaged_dependency_count}")
message(STATUS "System      : ${system_dependency_count}")
message(STATUS "External    : ${external_dependency_count}")
message(STATUS "Unresolved  : ${unresolved_dependency_count}")
message(STATUS "Conflicting : ${conflicting_dependency_count}")
message(STATUS "Report      : ${audit_report}")
message(STATUS "========================================")

if(external_dependencies
   OR unresolved_dependencies
   OR conflicting_dependency_names)
    message(WARNING
        "[runtime-audit] Potentially incomplete runtime dependency set. "
        "This audit is report-only; see ${audit_report}")
endif()

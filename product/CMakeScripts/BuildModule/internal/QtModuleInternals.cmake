include_guard()

include("${CMAKE_CURRENT_LIST_DIR}/../SetIDEFolder.cmake")

function(_tt_apply_qml_alias_mappings)
    set(one_value_args MODULE_NAME)
    set(multi_value_args QML_FILES ALIAS_MAPPINGS)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "" "${one_value_args}" "${multi_value_args}")

    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[BuildQtModule:${ARG_MODULE_NAME}] Unknown QML alias arguments: "
            "${ARG_UNPARSED_ARGUMENTS}")
    endif()

    set(mapped_qml_files)
    set(mapped_qml_aliases)

    foreach(mapping IN LISTS ARG_ALIAS_MAPPINGS)
        string(FIND "${mapping}" "|" separator_index)
        string(LENGTH "${mapping}" mapping_length)

        if(separator_index LESS 1)
            message(FATAL_ERROR
                "[BuildQtModule:${ARG_MODULE_NAME}] Invalid QML alias mapping "
                "'${mapping}'. Expected '<physical-file>|<resource-alias>'.")
        endif()

        math(EXPR alias_index "${separator_index} + 1")
        if(alias_index GREATER_EQUAL mapping_length)
            message(FATAL_ERROR
                "[BuildQtModule:${ARG_MODULE_NAME}] Empty resource alias in "
                "QML mapping '${mapping}'.")
        endif()

        string(SUBSTRING "${mapping}" 0 ${separator_index} qml_file)
        string(SUBSTRING "${mapping}" ${alias_index} -1 qml_alias)

        list(FIND ARG_QML_FILES "${qml_file}" qml_file_index)
        if(qml_file_index EQUAL -1)
            message(FATAL_ERROR
                "[BuildQtModule:${ARG_MODULE_NAME}] QML alias source "
                "'${qml_file}' is not listed in QML_TARGET_FILES.")
        endif()

        list(FIND mapped_qml_files "${qml_file}" duplicate_file_index)
        if(NOT duplicate_file_index EQUAL -1)
            message(FATAL_ERROR
                "[BuildQtModule:${ARG_MODULE_NAME}] QML file '${qml_file}' "
                "has more than one alias mapping.")
        endif()

        list(FIND mapped_qml_aliases "${qml_alias}" duplicate_alias_index)
        if(NOT duplicate_alias_index EQUAL -1)
            message(FATAL_ERROR
                "[BuildQtModule:${ARG_MODULE_NAME}] QML resource alias "
                "'${qml_alias}' is used more than once.")
        endif()

        list(APPEND mapped_qml_files "${qml_file}")
        list(APPEND mapped_qml_aliases "${qml_alias}")
        set_source_files_properties("${qml_file}" PROPERTIES
            QT_RESOURCE_ALIAS "${qml_alias}")
    endforeach()
endfunction()

function(_tt_prepare_qml_resources)
    set(one_value_args RESOURCE_DIRECTORY OUTPUT_VARIABLE)
    set(multi_value_args RESOURCES)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "" "${one_value_args}" "${multi_value_args}")

    if(NOT ARG_OUTPUT_VARIABLE)
        message(FATAL_ERROR
            "[_tt_prepare_qml_resources] OUTPUT_VARIABLE is required")
    endif()
    if(ARG_RESOURCES AND NOT ARG_RESOURCE_DIRECTORY)
        message(FATAL_ERROR
            "[_tt_prepare_qml_resources] RESOURCE_DIRECTORY is required when "
            "RESOURCES are specified")
    endif()
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[_tt_prepare_qml_resources] Unknown arguments: "
            "${ARG_UNPARSED_ARGUMENTS}")
    endif()

    set(prepared_resources)
    foreach(resource IN LISTS ARG_RESOURCES)
        set(resource_path "${ARG_RESOURCE_DIRECTORY}/${resource}")
        set_source_files_properties("${resource_path}" PROPERTIES
            QT_RESOURCE_ALIAS "${resource}")
        list(APPEND prepared_resources "${resource_path}")
    endforeach()

    set(${ARG_OUTPUT_VARIABLE} "${prepared_resources}" PARENT_SCOPE)
endfunction()

function(_tt_collect_qml_header_directories)
    set(one_value_args BASE_DIRECTORY OUTPUT_VARIABLE)
    set(multi_value_args SOURCES)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "" "${one_value_args}" "${multi_value_args}")

    if(NOT ARG_BASE_DIRECTORY)
        message(FATAL_ERROR
            "[_tt_collect_qml_header_directories] BASE_DIRECTORY is required")
    endif()
    if(NOT ARG_OUTPUT_VARIABLE)
        message(FATAL_ERROR
            "[_tt_collect_qml_header_directories] OUTPUT_VARIABLE is required")
    endif()
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[_tt_collect_qml_header_directories] Unknown arguments: "
            "${ARG_UNPARSED_ARGUMENTS}")
    endif()

    set(header_directories)
    foreach(source IN LISTS ARG_SOURCES)
        if(source MATCHES "\\.(h|hpp)$")
            set(header_path "${source}")
            if(IS_ABSOLUTE "${header_path}")
                cmake_path(NORMAL_PATH header_path)
            else()
                cmake_path(
                    ABSOLUTE_PATH header_path
                    BASE_DIRECTORY "${ARG_BASE_DIRECTORY}"
                    NORMALIZE
                )
            endif()

            cmake_path(GET header_path PARENT_PATH header_directory)
            list(APPEND header_directories "${header_directory}")
        endif()
    endforeach()

    list(REMOVE_DUPLICATES header_directories)
    set(${ARG_OUTPUT_VARIABLE} "${header_directories}" PARENT_SCOPE)
endfunction()

function(_tt_configure_qml_registration_includes)
    set(one_value_args MODULE_NAME)
    set(multi_value_args HEADER_DIRECTORIES)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "" "${one_value_args}" "${multi_value_args}")

    if(NOT ARG_MODULE_NAME)
        message(FATAL_ERROR
            "[_tt_configure_qml_registration_includes] MODULE_NAME is required")
    endif()
    if(NOT TARGET "${ARG_MODULE_NAME}")
        message(FATAL_ERROR
            "[_tt_configure_qml_registration_includes] Unknown target: "
            "${ARG_MODULE_NAME}")
    endif()
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[_tt_configure_qml_registration_includes] Unknown arguments: "
            "${ARG_UNPARSED_ARGUMENTS}")
    endif()

    foreach(header_directory IN LISTS ARG_HEADER_DIRECTORIES)
        target_include_directories("${ARG_MODULE_NAME}" PRIVATE
            "$<BUILD_INTERFACE:${header_directory}>")
    endforeach()

    if(TARGET "${ARG_MODULE_NAME}plugin")
        target_include_directories("${ARG_MODULE_NAME}plugin" PRIVATE
            "$<TARGET_PROPERTY:${ARG_MODULE_NAME},INCLUDE_DIRECTORIES>")
        foreach(header_directory IN LISTS ARG_HEADER_DIRECTORIES)
            target_include_directories("${ARG_MODULE_NAME}plugin" PRIVATE
                "$<BUILD_INTERFACE:${header_directory}>")
        endforeach()
    endif()
endfunction()

function(_tt_configure_qml_internal_target_folders)
    set(one_value_args MODULE_NAME IDE_FOLDER)
    cmake_parse_arguments(PARSE_ARGV 0 ARG "" "${one_value_args}" "")

    if(NOT ARG_MODULE_NAME)
        message(FATAL_ERROR
            "[_tt_configure_qml_internal_target_folders] MODULE_NAME is required")
    endif()
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[_tt_configure_qml_internal_target_folders] Unknown arguments: "
            "${ARG_UNPARSED_ARGUMENTS}")
    endif()

    if(ARG_IDE_FOLDER)
        set(internal_folder "${ARG_IDE_FOLDER}/internalTargets")
    else()
        set(internal_folder "internalTargets")
    endif()

    SetIDEFolder(
        TARGET_NAMES
            "${ARG_MODULE_NAME}plugin"
            "${ARG_MODULE_NAME}plugin_init"
            "${ARG_MODULE_NAME}_other_files"
            "${ARG_MODULE_NAME}_qmlcache"
            "${ARG_MODULE_NAME}_resources_1"
            "${ARG_MODULE_NAME}_resources_2"
            "${ARG_MODULE_NAME}_resources_3"
        FOLDER_NAME "${internal_folder}"
    )
endfunction()

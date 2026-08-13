include_guard()

# Append one leaf module's files and feature-specific dependencies to the
# aggregate UIView BuildQtModule arguments.
function(UIViewAppendBuildInputs)
    set(options)
    set(oneValueArgs QML_ALIAS_PREFIX)
    set(multiValueArgs
        SOURCES
        QML_FILES
        PRIVATE_INCLUDE_DIRS
        PRIVATE_DEPENDENCIES
    )

    cmake_parse_arguments(
        INPUT
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )

    if(INPUT_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[UIViewAppendBuildInputs] Unknown arguments: ${INPUT_UNPARSED_ARGUMENTS}")
    endif()

    if(NOT UIVIEW_BUILD_ROOT)
        message(FATAL_ERROR
            "[UIViewAppendBuildInputs] UIVIEW_BUILD_ROOT is not initialized")
    endif()

    if(INPUT_QML_FILES AND NOT INPUT_QML_ALIAS_PREFIX)
        message(FATAL_ERROR
            "[UIViewAppendBuildInputs] QML_ALIAS_PREFIX is required when QML_FILES are specified")
    endif()

    set(absolute_sources)
    foreach(source IN LISTS INPUT_SOURCES)
        cmake_path(
            ABSOLUTE_PATH source
            BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            NORMALIZE
            OUTPUT_VARIABLE absolute_source
        )
        list(APPEND absolute_sources "${absolute_source}")
    endforeach()

    set(absolute_include_dirs)
    foreach(include_dir IN LISTS INPUT_PRIVATE_INCLUDE_DIRS)
        cmake_path(
            ABSOLUTE_PATH include_dir
            BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            NORMALIZE
            OUTPUT_VARIABLE absolute_include_dir
        )
        list(APPEND absolute_include_dirs "${absolute_include_dir}")
    endforeach()

    set(absolute_qml_files)
    set(qml_alias_mappings)
    foreach(qml_file IN LISTS INPUT_QML_FILES)
        cmake_path(
            ABSOLUTE_PATH qml_file
            BASE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            NORMALIZE
            OUTPUT_VARIABLE absolute_qml_file
        )

        set(qml_alias "${INPUT_QML_ALIAS_PREFIX}")
        cmake_path(APPEND qml_alias "${qml_file}")
        cmake_path(NORMAL_PATH qml_alias)

        list(APPEND absolute_qml_files "${absolute_qml_file}")
        list(APPEND qml_alias_mappings "${absolute_qml_file}|${qml_alias}")
    endforeach()

    if(absolute_sources)
        set_property(
            DIRECTORY "${UIVIEW_BUILD_ROOT}"
            APPEND PROPERTY UIVIEW_BUILD_SOURCES
            ${absolute_sources}
        )
    endif()

    if(absolute_qml_files)
        set_property(
            DIRECTORY "${UIVIEW_BUILD_ROOT}"
            APPEND PROPERTY UIVIEW_BUILD_QML_FILES
            ${absolute_qml_files}
        )
        set_property(
            DIRECTORY "${UIVIEW_BUILD_ROOT}"
            APPEND PROPERTY UIVIEW_BUILD_QML_ALIAS_MAPPINGS
            ${qml_alias_mappings}
        )
    endif()

    if(absolute_include_dirs)
        set_property(
            DIRECTORY "${UIVIEW_BUILD_ROOT}"
            APPEND PROPERTY UIVIEW_BUILD_PRIVATE_INCLUDE_DIRS
            ${absolute_include_dirs}
        )
    endif()

    if(INPUT_PRIVATE_DEPENDENCIES)
        set_property(
            DIRECTORY "${UIVIEW_BUILD_ROOT}"
            APPEND PROPERTY UIVIEW_BUILD_PRIVATE_DEPENDENCIES
            ${INPUT_PRIVATE_DEPENDENCIES}
        )
    endif()
endfunction()

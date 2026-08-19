include_guard()

function(_tt_get_runtime_payload_registry_root output_variable)
    if(NOT TT_RUNTIME_PAYLOAD_REGISTRY_ROOT)
        message(FATAL_ERROR
            "[RuntimePayloadRegistry] "
            "TT_RUNTIME_PAYLOAD_REGISTRY_ROOT is not initialized")
    endif()
    if(NOT IS_DIRECTORY "${TT_RUNTIME_PAYLOAD_REGISTRY_ROOT}")
        message(FATAL_ERROR
            "[RuntimePayloadRegistry] Registry root does not exist: "
            "${TT_RUNTIME_PAYLOAD_REGISTRY_ROOT}")
    endif()

    cmake_path(
        NORMAL_PATH TT_RUNTIME_PAYLOAD_REGISTRY_ROOT
        OUTPUT_VARIABLE registry_root
    )
    set("${output_variable}" "${registry_root}" PARENT_SCOPE)
endfunction()

function(RegisterRuntimePayload)
    set(options REQUIRED USE_SOURCE_PERMISSIONS)
    set(one_value_args
        ID
        PRODUCER_TARGET
        TYPE
        SOURCE
        DESTINATION_NAME
        PLACEMENT
    )
    cmake_parse_arguments(
        PARSE_ARGV 0 PAYLOAD
        "${options}" "${one_value_args}" "")

    foreach(required_arg
        ID
        PRODUCER_TARGET
        TYPE
        SOURCE
        DESTINATION_NAME
        PLACEMENT
    )
        if(NOT PAYLOAD_${required_arg})
            message(FATAL_ERROR
                "[RegisterRuntimePayload] ${required_arg} is required")
        endif()
    endforeach()
    if(PAYLOAD_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR
            "[RegisterRuntimePayload:${PAYLOAD_ID}] Arguments missing values: "
            "${PAYLOAD_KEYWORDS_MISSING_VALUES}")
    endif()
    if(PAYLOAD_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[RegisterRuntimePayload:${PAYLOAD_ID}] Unknown arguments: "
            "${PAYLOAD_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT PAYLOAD_ID MATCHES "^[A-Za-z_][A-Za-z0-9_]*$")
        message(FATAL_ERROR
            "[RegisterRuntimePayload] ID must be a valid identifier: "
            "${PAYLOAD_ID}")
    endif()
    if(NOT TARGET "${PAYLOAD_PRODUCER_TARGET}")
        message(FATAL_ERROR
            "[RegisterRuntimePayload:${PAYLOAD_ID}] Producer target does not "
            "exist: ${PAYLOAD_PRODUCER_TARGET}")
    endif()
    if(NOT PAYLOAD_TYPE MATCHES "^(DIRECTORY|PROGRAM|FILE)$")
        message(FATAL_ERROR
            "[RegisterRuntimePayload:${PAYLOAD_ID}] Unsupported TYPE: "
            "${PAYLOAD_TYPE}")
    endif()
    if(NOT PAYLOAD_PLACEMENT STREQUAL "PRIVATE_TOOL")
        message(FATAL_ERROR
            "[RegisterRuntimePayload:${PAYLOAD_ID}] Unsupported PLACEMENT: "
            "${PAYLOAD_PLACEMENT}")
    endif()

    if(NOT PAYLOAD_DESTINATION_NAME MATCHES "^[A-Za-z0-9_.-]+$")
        message(FATAL_ERROR
            "[RegisterRuntimePayload:${PAYLOAD_ID}] DESTINATION_NAME must be "
            "one relative path component: ${PAYLOAD_DESTINATION_NAME}")
    endif()

    _tt_get_runtime_payload_registry_root(registry_root)

    get_property(payload_ids
        DIRECTORY "${registry_root}"
        PROPERTY TT_RUNTIME_PAYLOAD_IDS
    )
    if(PAYLOAD_ID IN_LIST payload_ids)
        message(FATAL_ERROR
            "[RegisterRuntimePayload] Duplicate payload ID: ${PAYLOAD_ID}")
    endif()

    get_filename_component(payload_source
        "${PAYLOAD_SOURCE}"
        ABSOLUTE
        BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
    )

    set_property(
        DIRECTORY "${registry_root}"
        APPEND PROPERTY TT_RUNTIME_PAYLOAD_IDS "${PAYLOAD_ID}"
    )
    foreach(field PRODUCER_TARGET TYPE DESTINATION_NAME PLACEMENT)
        set_property(
            DIRECTORY "${registry_root}"
            PROPERTY "TT_RUNTIME_PAYLOAD_${PAYLOAD_ID}_${field}"
                "${PAYLOAD_${field}}"
        )
    endforeach()
    set_property(
        DIRECTORY "${registry_root}"
        PROPERTY "TT_RUNTIME_PAYLOAD_${PAYLOAD_ID}_SOURCE"
            "${payload_source}"
    )
    set_property(
        DIRECTORY "${registry_root}"
        PROPERTY "TT_RUNTIME_PAYLOAD_${PAYLOAD_ID}_REQUIRED"
            "${PAYLOAD_REQUIRED}"
    )
    set_property(
        DIRECTORY "${registry_root}"
        PROPERTY "TT_RUNTIME_PAYLOAD_${PAYLOAD_ID}_USE_SOURCE_PERMISSIONS"
            "${PAYLOAD_USE_SOURCE_PERMISSIONS}"
    )

    message(STATUS
        "[RuntimePayloadRegistry] Registered ${PAYLOAD_ID} "
        "(${PAYLOAD_TYPE}, ${PAYLOAD_PLACEMENT})")
endfunction()

function(InstallRegisteredRuntimePayloads)
    set(one_value_args TARGET)
    cmake_parse_arguments(
        PARSE_ARGV 0 INSTALL_PAYLOADS "" "${one_value_args}" "")

    if(NOT INSTALL_PAYLOADS_TARGET)
        message(FATAL_ERROR
            "[InstallRegisteredRuntimePayloads] TARGET is required")
    endif()
    if(INSTALL_PAYLOADS_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR
            "[InstallRegisteredRuntimePayloads:${INSTALL_PAYLOADS_TARGET}] "
            "Arguments missing values: "
            "${INSTALL_PAYLOADS_KEYWORDS_MISSING_VALUES}")
    endif()
    if(INSTALL_PAYLOADS_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[InstallRegisteredRuntimePayloads:${INSTALL_PAYLOADS_TARGET}] "
            "Unknown arguments: ${INSTALL_PAYLOADS_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT TARGET "${INSTALL_PAYLOADS_TARGET}")
        message(FATAL_ERROR
            "[InstallRegisteredRuntimePayloads] Target does not exist: "
            "${INSTALL_PAYLOADS_TARGET}")
    endif()

    _tt_get_runtime_payload_registry_root(registry_root)
    get_property(payload_ids
        DIRECTORY "${registry_root}"
        PROPERTY TT_RUNTIME_PAYLOAD_IDS
    )

    if(NOT payload_ids)
        message(STATUS
            "[RuntimePayloadRegistry] No payloads registered for "
            "${INSTALL_PAYLOADS_TARGET}")
        return()
    endif()

    get_target_property(app_output_name
        "${INSTALL_PAYLOADS_TARGET}" OUTPUT_NAME)
    if(NOT app_output_name OR app_output_name MATCHES "-NOTFOUND$")
        set(app_output_name "${INSTALL_PAYLOADS_TARGET}")
    endif()

    if(APPLE)
        get_target_property(is_macos_bundle
            "${INSTALL_PAYLOADS_TARGET}" MACOSX_BUNDLE)
        if(NOT is_macos_bundle)
            message(FATAL_ERROR
                "[InstallRegisteredRuntimePayloads:${INSTALL_PAYLOADS_TARGET}] "
                "PRIVATE_TOOL payloads require a macOS bundle target")
        endif()
        set(private_tool_destination
            "bin/${app_output_name}.app/Contents/Resources")
    elseif(WIN32)
        set(private_tool_destination bin)
    else()
        set(private_tool_destination lib)
    endif()

    foreach(payload_id IN LISTS payload_ids)
        foreach(field
            PRODUCER_TARGET
            TYPE
            SOURCE
            DESTINATION_NAME
            PLACEMENT
            REQUIRED
            USE_SOURCE_PERMISSIONS
        )
            get_property(payload_${field}
                DIRECTORY "${registry_root}"
                PROPERTY "TT_RUNTIME_PAYLOAD_${payload_id}_${field}"
            )
        endforeach()

        add_dependencies(
            "${INSTALL_PAYLOADS_TARGET}"
            "${payload_PRODUCER_TARGET}"
        )

        if(payload_PLACEMENT STREQUAL "PRIVATE_TOOL")
            set(payload_destination "${private_tool_destination}")
        else()
            message(FATAL_ERROR
                "[InstallRegisteredRuntimePayloads:${payload_id}] "
                "Unsupported placement: ${payload_PLACEMENT}")
        endif()

        set(optional_argument)
        if(NOT payload_REQUIRED)
            list(APPEND optional_argument OPTIONAL)
        endif()
        if(payload_TYPE STREQUAL "DIRECTORY")
            set(permission_argument)
            if(payload_USE_SOURCE_PERMISSIONS)
                list(APPEND permission_argument USE_SOURCE_PERMISSIONS)
            endif()
            install(
                DIRECTORY "${payload_SOURCE}/"
                DESTINATION
                    "${payload_destination}/${payload_DESTINATION_NAME}"
                ${permission_argument}
                ${optional_argument}
            )
        elseif(payload_TYPE STREQUAL "PROGRAM")
            install(
                PROGRAMS "${payload_SOURCE}"
                DESTINATION "${payload_destination}"
                RENAME "${payload_DESTINATION_NAME}"
                ${optional_argument}
            )
        elseif(payload_TYPE STREQUAL "FILE")
            install(
                FILES "${payload_SOURCE}"
                DESTINATION "${payload_destination}"
                RENAME "${payload_DESTINATION_NAME}"
                ${optional_argument}
            )
        endif()

        message(STATUS
            "[RuntimePayloadRegistry] ${payload_id} -> "
            "${payload_destination}/${payload_DESTINATION_NAME}")
    endforeach()
endfunction()

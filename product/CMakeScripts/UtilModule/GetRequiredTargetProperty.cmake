include_guard()

function(get_required_target_property)
    set(one_value_args TARGET PROPERTY OUTPUT_VARIABLE)
    cmake_parse_arguments(
        PARSE_ARGV 0 GRTP "" "${one_value_args}" "")

    foreach(required_arg TARGET PROPERTY OUTPUT_VARIABLE)
        if(NOT GRTP_${required_arg})
            message(FATAL_ERROR
                "[get_required_target_property] ${required_arg} is required")
        endif()
    endforeach()
    if(GRTP_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR
            "[get_required_target_property] Arguments missing values: "
            "${GRTP_KEYWORDS_MISSING_VALUES}")
    endif()
    if(GRTP_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[get_required_target_property] Unknown arguments: "
            "${GRTP_UNPARSED_ARGUMENTS}")
    endif()
    if(NOT TARGET "${GRTP_TARGET}")
        message(FATAL_ERROR
            "[get_required_target_property] Target does not exist: "
            "${GRTP_TARGET}")
    endif()

    get_target_property(
        property_value "${GRTP_TARGET}" "${GRTP_PROPERTY}")
    if(property_value MATCHES "-NOTFOUND$"
       OR "${property_value}" STREQUAL "")
        message(FATAL_ERROR
            "[get_required_target_property] Target '${GRTP_TARGET}' does not "
            "provide required property '${GRTP_PROPERTY}'")
    endif()

    set("${GRTP_OUTPUT_VARIABLE}" "${property_value}" PARENT_SCOPE)
endfunction()

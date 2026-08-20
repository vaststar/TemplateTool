include_guard(GLOBAL)

# Apply TemplateTool's compile contract directly to a compiled first-party
# target. Third-party targets never call this function and therefore retain
# their own compiler settings.
function(ApplyProjectCompileOptions)
    set(options PUBLIC_CXX_STANDARD)
    set(one_value_args TARGET)
    cmake_parse_arguments(
        PARSE_ARGV 0 ARG "${options}" "${one_value_args}" "")

    if(NOT ARG_TARGET)
        message(FATAL_ERROR
            "[ApplyProjectCompileOptions] TARGET is required")
    endif()
    if(NOT TARGET "${ARG_TARGET}")
        message(FATAL_ERROR
            "[ApplyProjectCompileOptions] Unknown target: ${ARG_TARGET}")
    endif()
    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR
            "[ApplyProjectCompileOptions:${ARG_TARGET}] Unknown arguments: "
            "${ARG_UNPARSED_ARGUMENTS}")
    endif()

    get_target_property(is_imported "${ARG_TARGET}" IMPORTED)
    if(is_imported)
        message(FATAL_ERROR
            "[ApplyProjectCompileOptions:${ARG_TARGET}] Imported targets "
            "cannot use project compile options")
    endif()

    get_target_property(target_type "${ARG_TARGET}" TYPE)
    if(target_type STREQUAL "INTERFACE_LIBRARY"
       OR target_type STREQUAL "UTILITY")
        message(FATAL_ERROR
            "[ApplyProjectCompileOptions:${ARG_TARGET}] Target type "
            "${target_type} has no compiled project sources")
    endif()

    if(ARG_PUBLIC_CXX_STANDARD)
        set(cxx_standard_scope PUBLIC)
    else()
        set(cxx_standard_scope PRIVATE)
    endif()

    get_target_property(applied_scope
        "${ARG_TARGET}" TT_PROJECT_COMPILE_OPTIONS_SCOPE)
    if(applied_scope)
        if(NOT "${applied_scope}" STREQUAL "${cxx_standard_scope}")
            message(FATAL_ERROR
                "[ApplyProjectCompileOptions:${ARG_TARGET}] Compile options "
                "were already applied with ${applied_scope} C++ standard "
                "visibility; requested ${cxx_standard_scope}")
        endif()
        return()
    endif()

    target_compile_features("${ARG_TARGET}"
        ${cxx_standard_scope} cxx_std_20)

    target_compile_options("${ARG_TARGET}" PRIVATE
        "$<$<COMPILE_LANG_AND_ID:C,MSVC>:/utf-8>"
        "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/utf-8>"
        "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/Zc:__cplusplus>"
        "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/W4>"
        "$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wall>"
        "$<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang,AppleClang>:-Wextra>"
    )

    set_target_properties("${ARG_TARGET}" PROPERTIES
        CXX_EXTENSIONS OFF
        TT_PROJECT_COMPILE_OPTIONS_SCOPE "${cxx_standard_scope}"
    )
endfunction()

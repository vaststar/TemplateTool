function(GenerateAppVersionMeta)
    set(oneValueArgs VERSION_MAJOR VERSION_MINOR OUTPUT_FILE)
    cmake_parse_arguments(GAVM "" "${oneValueArgs}" "" ${ARGN})

    if(NOT GAVM_OUTPUT_FILE)
        message(FATAL_ERROR "[GenerateAppVersionMeta] OUTPUT_FILE is required")
    endif()

    set(FORCE_FILE "${CMAKE_CURRENT_BINARY_DIR}/force_generate_version.stamp")

    message(STATUS "build json")
    add_custom_target(generate_app_version_meta_always ALL
        # COMMAND ${CMAKE_COMMAND} -E touch ${OUTPUT_FILE}
        COMMAND ${CMAKE_COMMAND}
            -DVERSION_MAJOR=${GAVM_VERSION_MAJOR}
            -DVERSION_MINOR=${GAVM_VERSION_MINOR}
            -DOUTPUT_FILE=${GAVM_OUTPUT_FILE}
            -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/GenerateAppVersionMeta.cmake"
        # COMMENT "Generating version meta JSON: ${GAVM_OUTPUT_FILE}"
        # BYPRODUCTS "${GAVM_OUTPUT_FILE}"
        # VERBATIM
    )
    # set_target_properties(generate_app_version_meta_always PROPERTIES FOLDER codegen)
endfunction()

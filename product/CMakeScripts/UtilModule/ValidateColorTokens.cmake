# === ValidateColorTokens.cmake ===
# 提供 validate_color_tokens() 函数，用于在构建前校验 token.json 与 theme JSON 的一致性。
include_guard()

function(validate_color_tokens)
    set(oneValueArgs TOKEN_FILE THEME_DIR PALETTE_FILE SCRIPT)
    cmake_parse_arguments(VCT "" "${oneValueArgs}" "" ${ARGN})

    foreach(required_arg TOKEN_FILE THEME_DIR SCRIPT)
        if(NOT VCT_${required_arg})
            message(FATAL_ERROR "[validate_color_tokens] Missing: ${required_arg}")
        endif()
    endforeach()

    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    file(GLOB THEME_FILES "${VCT_THEME_DIR}/*.json")

    set(CMD ${Python3_EXECUTABLE} ${VCT_SCRIPT}
        --token "${VCT_TOKEN_FILE}" --themes ${THEME_FILES})
    if(VCT_PALETTE_FILE)
        list(APPEND CMD --palette "${VCT_PALETTE_FILE}")
    endif()

    set(STAMP "${CMAKE_BINARY_DIR}/token_validation.stamp")
    add_custom_command(OUTPUT ${STAMP}
        COMMAND ${CMD}
        COMMAND ${CMAKE_COMMAND} -E touch ${STAMP}
        DEPENDS ${VCT_TOKEN_FILE} ${THEME_FILES} ${VCT_SCRIPT}
        COMMENT "Validating color token consistency..."
        VERBATIM)

    set(TARGET_NAME ValidateColorTokens)
    if(NOT TARGET ${TARGET_NAME})
        add_custom_target(${TARGET_NAME} ALL DEPENDS ${STAMP})
        set_target_properties(${TARGET_NAME} PROPERTIES FOLDER codegen)
    endif()

    list(LENGTH VCT_UNPARSED_ARGUMENTS unparsed_count)
    if(unparsed_count EQUAL 1)
        list(GET VCT_UNPARSED_ARGUMENTS 0 output_variable)
        set(${output_variable} ${TARGET_NAME} PARENT_SCOPE)
    endif()
endfunction()

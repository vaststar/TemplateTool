# === ValidateFontTokens.cmake ===
# 提供 validate_font_tokens() 函数，用于在构建前校验 token.json、theme JSON 与 palette.json 的一致性。
include_guard()

function(validate_font_tokens)
    set(oneValueArgs TOKEN_FILE THEME_DIR PALETTE_FILE SCRIPT)
    cmake_parse_arguments(VFT "" "${oneValueArgs}" "" ${ARGN})

    foreach(required_arg TOKEN_FILE THEME_DIR PALETTE_FILE SCRIPT)
        if(NOT VFT_${required_arg})
            message(FATAL_ERROR "[validate_font_tokens] Missing: ${required_arg}")
        endif()
    endforeach()

    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    file(GLOB THEME_FILES "${VFT_THEME_DIR}/*.json")

    set(CMD ${Python3_EXECUTABLE} ${VFT_SCRIPT}
        --token "${VFT_TOKEN_FILE}" --themes ${THEME_FILES}
        --palette "${VFT_PALETTE_FILE}")

    set(STAMP "${CMAKE_BINARY_DIR}/font_token_validation.stamp")
    add_custom_command(OUTPUT ${STAMP}
        COMMAND ${CMD}
        COMMAND ${CMAKE_COMMAND} -E touch ${STAMP}
        DEPENDS ${VFT_TOKEN_FILE} ${THEME_FILES} ${VFT_PALETTE_FILE} ${VFT_SCRIPT}
        COMMENT "Validating font token consistency..."
        VERBATIM)

    set(TARGET_NAME ValidateFontTokens)
    if(NOT TARGET ${TARGET_NAME})
        add_custom_target(${TARGET_NAME} ALL DEPENDS ${STAMP})
        set_target_properties(${TARGET_NAME} PROPERTIES FOLDER codegen)
    endif()

    list(LENGTH VFT_UNPARSED_ARGUMENTS unparsed_count)
    if(unparsed_count EQUAL 1)
        list(GET VFT_UNPARSED_ARGUMENTS 0 output_variable)
        set(${output_variable} ${TARGET_NAME} PARENT_SCOPE)
    endif()
endfunction()

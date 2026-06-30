# === ValidateAssetTokens.cmake ===
# 提供 validate_asset_tokens() 函数，用于在构建前校验 token.json 与 theme JSON 的一致性，
# 并验证 theme 中引用的资源文件是否真实存在于磁盘上。
include_guard()

function(validate_asset_tokens)
    set(oneValueArgs TOKEN_FILE THEME_DIR ASSET_ROOT SCRIPT)
    cmake_parse_arguments(VAT "" "${oneValueArgs}" "" ${ARGN})

    foreach(required_arg TOKEN_FILE THEME_DIR ASSET_ROOT SCRIPT)
        if(NOT VAT_${required_arg})
            message(FATAL_ERROR "[validate_asset_tokens] Missing: ${required_arg}")
        endif()
    endforeach()

    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    file(GLOB THEME_FILES "${VAT_THEME_DIR}/*.json")

    set(CMD ${Python3_EXECUTABLE} ${VAT_SCRIPT}
        --token "${VAT_TOKEN_FILE}" --themes ${THEME_FILES}
        --asset-root "${VAT_ASSET_ROOT}")

    set(STAMP "${CMAKE_BINARY_DIR}/asset_token_validation.stamp")
    add_custom_command(OUTPUT ${STAMP}
        COMMAND ${CMD}
        COMMAND ${CMAKE_COMMAND} -E touch ${STAMP}
        DEPENDS ${VAT_TOKEN_FILE} ${THEME_FILES} ${VAT_SCRIPT}
        COMMENT "Validating asset token consistency and on-disk assets..."
        VERBATIM)

    set(TARGET_NAME ValidateAssetTokens)
    if(NOT TARGET ${TARGET_NAME})
        add_custom_target(${TARGET_NAME} ALL DEPENDS ${STAMP})
        set_target_properties(${TARGET_NAME} PROPERTIES FOLDER codegen)
    endif()

    list(LENGTH VAT_UNPARSED_ARGUMENTS unparsed_count)
    if(unparsed_count EQUAL 1)
        list(GET VAT_UNPARSED_ARGUMENTS 0 output_variable)
        set(${output_variable} ${TARGET_NAME} PARENT_SCOPE)
    endif()
endfunction()

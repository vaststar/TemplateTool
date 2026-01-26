add_subdirectory(QR-Code-generator)
if (TARGET qrcodegen)
    set_target_properties(qrcodegen PROPERTIES FOLDER framework/thirdparty/qrcodegen)
endif()
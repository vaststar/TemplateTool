set(TT_APPLICATION_TARGET TemplateTool::Application)

if(NOT TARGET "${TT_APPLICATION_TARGET}")
    message(FATAL_ERROR
        "[packaging] Application target does not exist: "
        "${TT_APPLICATION_TARGET}")
endif()

get_target_property(TT_APPLICATION_REAL_TARGET
    "${TT_APPLICATION_TARGET}" ALIASED_TARGET)
if(NOT TT_APPLICATION_REAL_TARGET
   OR TT_APPLICATION_REAL_TARGET MATCHES "-NOTFOUND$")
    set(TT_APPLICATION_REAL_TARGET "${TT_APPLICATION_TARGET}")
endif()

get_target_property(TT_APPLICATION_EXECUTABLE_NAME
    "${TT_APPLICATION_REAL_TARGET}" OUTPUT_NAME)
if(NOT TT_APPLICATION_EXECUTABLE_NAME
   OR TT_APPLICATION_EXECUTABLE_NAME MATCHES "-NOTFOUND$")
    set(TT_APPLICATION_EXECUTABLE_NAME "${TT_APPLICATION_REAL_TARGET}")
endif()

set(TT_PACKAGING_QT_DEPLOY_TARGETS
    TemplateTool::QtDeploymentRoot
)
get_filename_component(TT_PACKAGING_QML_SOURCE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/../application/qt"
    ABSOLUTE
)
set(TT_PACKAGING_PROGRAM_PAYLOADS
    ffmpeg
    ffprobe
)

if(NOT IS_DIRECTORY "${TT_PACKAGING_QML_SOURCE_DIR}")
    message(FATAL_ERROR
        "[packaging] QML source directory does not exist: "
        "${TT_PACKAGING_QML_SOURCE_DIR}")
endif()

add_subdirectory(runtime)
add_subdirectory(distribution)

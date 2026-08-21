# macOS: Fix AGL framework issue - AGL is deprecated and removed from newer SDKs
# Qt's FindWrapOpenGL.cmake tries to link AGL which doesn't exist in modern macOS SDK
# We create WrapOpenGL target ourselves without AGL before Qt can find it
if(APPLE)
    find_package(OpenGL QUIET)
    if(OpenGL_FOUND AND NOT TARGET WrapOpenGL::WrapOpenGL)
        add_library(WrapOpenGL::WrapOpenGL INTERFACE IMPORTED)
        # Link only OpenGL framework, not AGL (which is deprecated/removed)
        target_link_libraries(WrapOpenGL::WrapOpenGL INTERFACE "-framework OpenGL")
        set(WrapOpenGL_FOUND ON CACHE BOOL "WrapOpenGL found" FORCE)
    endif()
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Quick Qml Gui Widgets LinguistTools Multimedia)

qt_standard_project_setup(REQUIRES 6.5)

set(QT_TRANSLATION_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resource/translations)
set(QT_TRANSLATION_CODE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(UIAuxiliary)
add_subdirectory(UIComponent)
add_subdirectory(UIPresentation)
add_subdirectory(AppUI)
add_subdirectory(mainUI)

if(TARGET all_qmllint)
    set_target_properties(all_qmllint PROPERTIES FOLDER UI)
endif()

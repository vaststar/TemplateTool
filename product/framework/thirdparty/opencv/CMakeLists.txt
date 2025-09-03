if(WIN32)
    set(OpenCV_DIR "${CMAKE_CURRENT_SOURCE_DIR}/windows/x64/vc16/lib")
endif()

find_package(OpenCV REQUIRED)

message(STATUS "OpenCV_INCLUDE_DIRS: ${OpenCV_INCLUDE_DIRS}")
add_library(opencv INTERFACE)
target_include_directories(opencv INTERFACE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(opencv INTERFACE ${OpenCV_LIBS})

if(WIN32)
    set(OPENCV_DLL_RELEASE ${OpenCV_DIR}/../bin/opencv_world4120.dll)
    set(OPENCV_LIB_RELEASE ${OpenCV_DIR}/opencv_world4120.lib)
    set(OPENCV_PDB_RELEASE ${OpenCV_DIR}/../bin/opencv_world4120.pdb)

    set(OPENCV_DLL_DEBUG ${OpenCV_DIR}/../bin/opencv_world4120d.dll)
    set(OPENCV_LIB_DEBUG ${OpenCV_DIR}/opencv_world4120d.lib)
    set(OPENCV_PDB_DEBUG ${OpenCV_DIR}/../bin/opencv_world4120d.pdb)

    add_custom_target(copy_opencv)
    set_target_properties(copy_opencv PROPERTIES FOLDER framework/thirdparty/opencv)

    add_custom_command( TARGET copy_opencv POST_BUILD  COMMAND_EXPAND_LISTS
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<$<CONFIG:Release>:Release>$<$<CONFIG:Debug>:Debug>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<$<CONFIG:Release>:${OPENCV_DLL_RELEASE}>$<$<CONFIG:Debug>:${OPENCV_DLL_DEBUG}>" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<$<CONFIG:Release>:Release>$<$<CONFIG:Debug>:Debug>/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<$<CONFIG:Release>:${OPENCV_LIB_RELEASE}>$<$<CONFIG:Debug>:${OPENCV_LIB_DEBUG}>" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<$<CONFIG:Release>:Release>$<$<CONFIG:Debug>:Debug>/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "$<$<CONFIG:Release>:${OPENCV_PDB_RELEASE}>$<$<CONFIG:Debug>:${OPENCV_PDB_DEBUG}>" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<$<CONFIG:Release>:Release>$<$<CONFIG:Debug>:Debug>/"
    )
    add_dependencies(opencv copy_opencv)
endif()
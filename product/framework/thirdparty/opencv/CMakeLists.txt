set(OpenCV_DIR "${CMAKE_CURRENT_SOURCE_DIR}/windows/x64/vc17/lib")
set(OPENCV_MAP_IMPORTED_CONFIG "")
find_package(OpenCV REQUIRED)

set(OPENCV_DLL_FILES)
file(GLOB OPENCV_DLL_FILES ${OpenCV_DIR}/../bin/*.dll)
file(GLOB OPENCV_LIB_FILES ${OpenCV_DIR}/*.lib)

add_custom_target(copy_opencv)
set_target_properties(copy_opencv PROPERTIES FOLDER thirdparty/copy_opencv)

add_custom_command( TARGET copy_opencv POST_BUILD  COMMAND_EXPAND_LISTS
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug"
    COMMAND ${CMAKE_COMMAND} -E copy "$<$<CONFIG:Release>:${OPENCV_DLL_FILES}>$<$<CONFIG:Debug>:${OPENCV_DLL_FILES}>" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<$<CONFIG:Release>:Release>$<$<CONFIG:Debug>:Debug>/"
    COMMAND ${CMAKE_COMMAND} -E copy "$<$<CONFIG:Release>:${OPENCV_LIB_FILES}>$<$<CONFIG:Debug>:${OPENCV_LIB_FILES}>" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<$<CONFIG:Release>:Release>$<$<CONFIG:Debug>:Debug>/"
)  
message(STATUS "OpenCV_INCLUDE_DIRS: ${OpenCV_INCLUDE_DIRS}")
add_library(opencv INTERFACE)
target_include_directories(opencv INTERFACE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(opencv INTERFACE ${OpenCV_LIBS})
add_dependencies(opencv copy_opencv)
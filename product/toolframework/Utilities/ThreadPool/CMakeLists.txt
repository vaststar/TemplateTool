include(BuildUtilities)

BuildUtilities(
    MODULE_NAME 
        ThreadPool
    TARGET_SOURCE 
        include/ThreadPoolExport.h
        include/ThreadPool.h
        include/ThreadPoolControl.h
        include/ThreadPoolUtil.h
        src/ThreadPool.cpp
        src/ThreadPoolControl.cpp
        src/ThreadPoolUtil.cpp
    INSTALL_PUBLIC_HEADER 
        include/ThreadPoolExport.h 
        include/ThreadPoolUtil.h
)

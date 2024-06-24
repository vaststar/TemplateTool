# Thread Pool using C++
A very nice thread pool in c++20 written by Thomas Zhu.

# Base ThreadPool instructions
export usage: ThreadPoolUtil, user should use this header to develop

core class: ThreadPool provide a core function of thread pool
* `#include "ThreadPool.h"`
* create threadpool object, `ThreadPool pool(4,"testpool");`
* push your function, `pool.enqueueFunc("test func",0,std::bind(functionname,params));`
* Or get future function `auto temp = pool.enqueueFutureFunc("test func",0,functionname,params); auto result = temp.get();`

# Requirement
* >= c++20
* >= cmake3.24

# Install
* make and cd in `build` dir
* run command: `cmake .. && make && make install`

# Test
* make and cd in `build` dir
* for cmake test, run command: `cmake .. && make && ctest -V`
* for catch2 test, run command: `cmake .. && make` and run `testThreadPool_catch2`

# Usage
* firstly, link library in your project-set
* write some common code in your codings

## Using in project
* install first
* set dir in your CMakeList,eg `set(ThreadPool_DIR $logInstalledDir$/ThreadPool/install/Release/cmake) `
* use find package , `find_package(ThreadPool REQUIRED)`
* include headers, `include_directories(${ThreadPool_INCLUDE_DIRS})`
* link target to you library, `target_link_libraries(testProgram ThreadPoolExport::ThreadPool)`


## Use in your code
* include header `#include "ThreadPoolUtil"`
* init thread pool `ThreadPoolUtil::initThreadPool(500, "default-pool-name")`
* create and run thread `ThreadPoolUtil::createThreadTask(std::bind(functionname,params), ThreadLevel::Level_Normal, "default-pool-name", "default-function-tag")`




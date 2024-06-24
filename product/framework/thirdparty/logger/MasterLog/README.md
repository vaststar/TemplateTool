# Thread-Safe log using C++
A very nice log in c++20 written by Thomas Zhu.

# Usage
* firstly, link library in your project-set
* write some common code in your codings

## Using in project
* install first
* set dir in your CMakeList,eg `set(MasterLog_DIR $logInstalledDir$/MasterLog/install/Release/cmake) `
* use find package , `find_package(MasterLog REQUIRED)`
* include headers, `include_directories(${MasterLog_INCLUDE_DIRS})`
* link target to you library, `target_link_libraries(testProgram MasterLogExport::MasterLog)`

## Using in code
```c++
#include <MasterLog/LogExport.h>
//define your configure, print all level log
auto configure = std::make_shared<LogLogSpace::LoggerConsoleConfigure>(MasterLogUtil::ALL_LOG_LEVEL);
MasterLogUtil::InitLogger({configure});
//define your log macro
#define TEST_CONSOLE_LOG_DEBUG(message) LOG_DEBUG("TEST_LOG",message,MasterLogUtil::Console_Logger_Name)
//usage

TEST_CONSOLE_LOG_DEBUG("startTest");
```
```c++
#include <MasterLog/LogExport.h>
//define your configure, eg. store at ./testMkdir/teestDD/testFile-2022-03-04.log
#define TEST_DEFAULT_FILE_LOG_DEBUG(message) LOG_DEBUG("TEST_DEFAULT_FILE_LOG",message,MasterLogUtil::Default_File_Logger_Name)
#define TEST_APP_FILE_LOG_DEBUG(message) LOG_DEBUG("TEST_APP_FILE_LOG",message,"APP")
std::string dirPath = "./testMakeDir/testFile";
std::string baseFileName = "default_log";
auto configure = std::make_shared<LogLogSpace::LoggerFileConfigure>(MasterLogUtil::ALL_LOG_LEVEL, dirPath,baseFileName, 180, 20*1024, MasterLogUtil::Default_File_Logger_Name);
std::string baseFileName_app = "app";
auto configure_app = std::make_shared<LogLogSpace::LoggerFileConfigure>(MasterLogUtil::ALL_LOG_LEVEL, dirPath,baseFileName_app, 180, 20*1024, "APP");
MasterLogUtil::InitLogger({configure, configure_app});
//usage
TEST_DEFAULT_FILE_LOG_DEBUG("to default_log");
TEST_APP_FILE_LOG_DEBUG("to app log");
```

# Requirement
* >= c++20
* >= cmake3.24

# Install
* make and cd in `build` dir
* run command: `cmake .. && make && make install`

# Test
* make and cd in `build` dir
* for cmake test, run command: `cmake .. && make && ctest -V`
* for catch2 test, run command: `cmake .. && make` and run testLog_catch2

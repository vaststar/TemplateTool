#include <catch2/catch_test_macros.hpp>
#include "LogExport.h"

#include <iostream>

#include <list>
#include <thread>
#include <chrono>

void testConsoleLog()
{
	auto configure = std::make_shared<LogLogSpace::LoggerConsoleConfigure>(MasterLogUtil::ALL_LOG_LEVEL, MasterLogUtil::Console_Logger_Name);
#define TEST_CONSOLE_LOG_DEBUG(message) LOG_DEBUG("TEST_LOG",message,MasterLogUtil::Console_Logger_Name)
	MasterLogUtil::InitLogger({configure});
	TEST_CONSOLE_LOG_DEBUG("startTest");
	std::list<std::thread> aaa;
	for (int i = 0;i < 3000 ;++i)
	{
		//TEST_LOG_DEBUG("test: "<<i);
		aaa.emplace_back([i]() {
			TEST_CONSOLE_LOG_DEBUG(i);
		});
	}
	for (auto it = aaa.begin();it != aaa.end();++it)
	{
		(*it).join();
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void testFileLog()
{
#define TEST_DEFAULT_FILE_LOG_DEBUG(message) LOG_DEBUG("TEST_DEFAULT_FILE_LOG",message,MasterLogUtil::Default_File_Logger_Name)
#define TEST_APP_FILE_LOG_DEBUG(message) LOG_DEBUG("TEST_APP_FILE_LOG",message,"APP")
	std::string dirPath = "./testMakeDir/testFile";
	std::string baseFileName = "default_log";
	auto configure = std::make_shared<LogLogSpace::LoggerFileConfigure>(MasterLogUtil::ALL_LOG_LEVEL, dirPath, baseFileName, 180, 20*1024, MasterLogUtil::Default_File_Logger_Name);
	std::string baseFileName_app = "app";
	auto configure_app = std::make_shared<LogLogSpace::LoggerFileConfigure>(MasterLogUtil::ALL_LOG_LEVEL, dirPath, baseFileName_app, 180, 20*1024, "APP");
	MasterLogUtil::InitLogger({configure, configure_app});
	std::list<std::thread> aaa;
	for (int i = 0;i < 2000 ;++i)
	{
		//TEST_LOG_DEBUG("test: "<<i);
		aaa.emplace_back([i]() {
			TEST_DEFAULT_FILE_LOG_DEBUG(i);
			TEST_APP_FILE_LOG_DEBUG(i*i);
		});
	}
	for (auto it = aaa.begin();it != aaa.end();++it)
	{
		(*it).join();
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

}

TEST_CASE( "testConsoleLog", "[MasterLogTests]" ) {
    testConsoleLog();
    REQUIRE( true );
}

TEST_CASE( "testFileLog", "[MasterLogTests]" ) {
	testFileLog();
    REQUIRE( true );
}
#include <catch2/catch_test_macros.hpp>

#include "ThreadPool/ThreadPoolUtil.h"

#include <vector>
#include <string>
#include <iostream>
#include <functional>

void threadPool_export_util_api()
{
	auto testFunc = [](int i){/*std::this_thread::sleep_for(std::chrono::milliseconds(100));*/ std::cout << i*i<<std::endl; };
	ThreadPoolUtil::initThreadPool(3);
	for (int i = 2000; i < 3000; ++i) {
		ThreadPoolUtil::createThreadTask(std::bind(testFunc,i), ThreadPoolUtil::ThreadLevel::Level_Normal);
	}
	for (int i = 3100; i < 3400; ++i) {
		ThreadPoolUtil::createThreadTask(std::bind(testFunc,i), ThreadPoolUtil::ThreadLevel::Level_High);
	}
}

TEST_CASE("all platform thread pool","[testThreadPool]")
{
	SECTION("test util api")
	{
		threadPool_export_util_api();
	}
}
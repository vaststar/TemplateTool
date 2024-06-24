#include <catch2/catch_test_macros.hpp>

#ifndef _WIN32
#include "ThreadPool.h"
#endif
#include "ThreadPool/ThreadPoolUtil.h"

#include <vector>
#include <string>
#include <iostream>
#include <functional>

#ifndef _WIN32
void threadPool_test_future_api()
{
	auto testFunc = [](int i)->int{/*std::this_thread::sleep_for(std::chrono::milliseconds(100));*/ return i*i;};
	ThreadPoolSpace::ThreadPool pool(3,"future-mode");
	//test future mode
	std::vector<std::future<int>> test;
	for (int i = 0; i < 200; ++i) {
		test.push_back(pool.enqueueFutureFunc("testFutureFunc-"+std::to_string(i),1, testFunc,i));
	}
	for (int i = 300; i < 400; ++i) {
		test.push_back(pool.enqueueFutureFunc("urgent_testFutureFunc-"+std::to_string(i),0, testFunc,i));
	}
	for(auto &tt : test)
	{
		std::cout<<tt.get()<<std::endl;
	}
}

void threadPool_test_normal_api()
{
	auto testFunc = [](int i){/*std::this_thread::sleep_for(std::chrono::milliseconds(100));*/ std::cout << i*i<<std::endl; };
	ThreadPoolSpace::ThreadPool pool(3,"mormal-mode");
	//test future mode
	std::vector<std::future<int>> test;
	for (int i = 1000; i < 1200; ++i) {
		pool.enqueueFunc("testNormalFunc-"+std::to_string(i),1, std::bind(testFunc,i));
	}
	for (int i = 1300; i < 1440; ++i) {
		pool.enqueueFunc("urgent_testNormalFunc-"+std::to_string(i),0, std::bind(testFunc,i));
	}
}
TEST_CASE("non-win thread pool","[testThreadPool]")
{
	SECTION("test future api")
	{
		threadPool_test_future_api();
	}

	SECTION("test normal api")
	{
		threadPool_test_normal_api();
	}
}
#endif

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
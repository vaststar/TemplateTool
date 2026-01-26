#include <catch2/catch_test_macros.hpp>

#include "ThreadPool/IThreadPool.h"

#include <vector>
#include <string>
#include <iostream>
#include <functional>

void threadPool_test_submit_api()
{
	auto pool = ThreadPool::IThreadPool::create(3, "test-pool");
	auto testFunc = [](int i){ std::cout << i*i << std::endl; };
	
	for (int i = 2000; i < 2100; ++i) {
		pool->submit(std::bind(testFunc, i), ThreadPool::Priority::Normal);
	}
	for (int i = 3100; i < 3200; ++i) {
		pool->submit(std::bind(testFunc, i), ThreadPool::Priority::High);
	}
}

void threadPool_test_future_api()
{
	auto pool = ThreadPool::IThreadPool::create(3, "future-pool");
	auto testFunc = [](int i) -> int { return i * i; };
	
	std::vector<std::future<int>> futures;
	for (int i = 0; i < 50; ++i) {
		futures.push_back(pool->submitWithFuturePriority(ThreadPool::Priority::Normal, testFunc, i));
	}
	for (int i = 100; i < 120; ++i) {
		futures.push_back(pool->submitWithFuturePriority(ThreadPool::Priority::Urgent, testFunc, i));
	}
	
	for (auto& f : futures) {
		std::cout << f.get() << std::endl;
	}
}

TEST_CASE("ThreadPool submit API", "[testThreadPool]")
{
	SECTION("test submit")
	{
		threadPool_test_submit_api();
	}
	
	SECTION("test future")
	{
		threadPool_test_future_api();
	}
}
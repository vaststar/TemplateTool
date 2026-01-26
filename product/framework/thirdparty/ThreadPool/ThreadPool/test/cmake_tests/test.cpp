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

int main(int argc, char* argv[])
{
	threadPool_test_submit_api();
	return 0;
}
#include "ThreadPool/IThreadPool.h"

#include <vector>
#include <string>
#include <iostream>
#include <functional>

void threadPool_test_future_api()
{
	auto pool = ThreadPool::IThreadPool::create(3, "future-mode");
	auto testFunc = [](int i) -> int { return i * i; };
	
	std::vector<std::future<int>> futures;
	for (int i = 0; i < 200; ++i) {
		futures.push_back(pool->submitWithFuturePriority(ThreadPool::Priority::Normal, testFunc, i));
	}
	for (int i = 300; i < 400; ++i) {
		futures.push_back(pool->submitWithFuturePriority(ThreadPool::Priority::Urgent, testFunc, i));
	}
	for (auto& f : futures) {
		std::cout << f.get() << std::endl;
	}
}

void threadPool_test_submit_api()
{
	auto pool = ThreadPool::IThreadPool::create(3, "normal-mode");
	auto testFunc = [](int i){ std::cout << i*i << std::endl; };
	
	for (int i = 1000; i < 1200; ++i) {
		pool->submit(std::bind(testFunc, i), ThreadPool::Priority::Normal);
	}
	for (int i = 1300; i < 1440; ++i) {
		pool->submit(std::bind(testFunc, i), ThreadPool::Priority::Urgent);
	}
}

int main(int argc, char* argv[])
{
	if (argc >= 2)
	{
		if (std::string(argv[1]) == "future")
		{
			threadPool_test_future_api();
		}
		else if (std::string(argv[1]) == "submit")
		{
			threadPool_test_submit_api();
		}
	}
	else
	{
		threadPool_test_submit_api();
	}
	return 0;
}
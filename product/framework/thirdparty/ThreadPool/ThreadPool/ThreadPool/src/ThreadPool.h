#ifndef ThreadPool_h__
#define ThreadPool_h__

#include <future>
#include <functional>

namespace ThreadPoolSpace{
class ThreadPool final
{
public:
	ThreadPool(uint32_t maxpool, const std::string& poolName);
	ThreadPool(ThreadPool &&) = delete;
	ThreadPool(const ThreadPool &) = delete;
	ThreadPool& operator=(const ThreadPool &) = delete;
	ThreadPool& operator=(ThreadPool &&) = delete;
	~ThreadPool();
public:
	/*enqueue fuction and get std::future result to wait*/
	template<typename Fun, typename... Args>
	auto enqueueFutureFunc(const std::string& functionTag, uint32_t urgentLevel, Fun &&f, Args&&... args)->std::future<decltype( std::declval<Fun>()( std::declval<Args>()... ) )>;
	/*just enqueue function*/
	void enqueueFunc(const std::string& functionTag, uint32_t urgentLevel, std::function<void()> func);
private:
	void initPool(uint32_t poolNumber);
	void pushFuncPri(const std::string& functionTag, uint32_t urgentLevel, std::function<void()>&&);
private:
	class DataPrivate;
	std::unique_ptr<DataPrivate> _p;
};

template<typename Fun, typename... Args>
auto ThreadPool::enqueueFutureFunc(const std::string& functionTag, uint32_t urgentLevel, Fun &&f, Args&&... args) ->std::future<decltype( std::declval<Fun>()( std::declval<Args>()... ) )>
{
	using return_type = decltype( std::declval<Fun>()( std::declval<Args>()... ) );

	// 将任务函数和其参数绑定，构建一个packaged_task
	auto task = std::make_shared< std::packaged_task<return_type()> >(
		std::bind(std::forward<Fun>(f), std::forward<Args>(args)...)
		);
	// 获取任务的future
	std::future<return_type> res = task->get_future();
	pushFuncPri(functionTag, urgentLevel, [task] {(*task)();});
	return res;
}
}
#endif // ThreadPool_h__
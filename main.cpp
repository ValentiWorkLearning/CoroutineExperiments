#include <coroutine>
#include <cstdint>
#include <chrono>
#include <thread>
#include <atomic>
#include <cstdio>
#include <tuple>
#include <vector>
#include <array>

//
//#include <cppcoro/task.hpp>
//#include <cppcoro/when_all.hpp>

//from cppcoro
class WhenAllCounter
{
public:

	WhenAllCounter(std::size_t count) noexcept
		: m_count(count + 1)
		, m_awaitingCoroutine(nullptr)
	{}

	bool is_ready() const noexcept
	{
		// We consider this complete if we're asking whether it's ready
		// after a coroutine has already been registered.
		return static_cast<bool>(m_awaitingCoroutine);
	}

	bool try_await(std::coroutine_handle<> awaitingCoroutine) noexcept
	{
		m_awaitingCoroutine = awaitingCoroutine;
		return m_count.fetch_sub(1, std::memory_order_acq_rel) > 1;
	}

	void notify_awaitable_completed() noexcept
	{
		if (m_count.fetch_sub(1, std::memory_order_acq_rel) == 1)
		{
			m_awaitingCoroutine.resume();
		}
	}

protected:

	std::atomic<std::size_t> m_count;
	std::coroutine_handle<> m_awaitingCoroutine;

};

struct Promise;
struct TransmitTask
{
	explicit constexpr TransmitTask(std::uint8_t command)
		:m_commandData{ command }
	{
	}

	void await_resume()
	{
		printf("Await Resume Transmit \n");
	}

	bool await_ready()
	{
		printf("Await Ready Transmit\n");
		return false;
	}

	void await_suspend(std::coroutine_handle<> _coroHandle)
	{
		printf("Await suspend Transmit\n");
		using namespace std::chrono_literals;
		auto pthread = std::thread([_coroHandle] { std::this_thread::sleep_for(2100ms); printf("Task completed!\n"); _coroHandle.resume(); });
		pthread.detach();
		
	}

	void launch(WhenAllCounter* _pCounter)
	{
		printf("Started task!\n");
		using namespace std::chrono_literals;
		auto pthread = std::thread([_pCounter] { std::this_thread::sleep_for(2100ms); printf("Task completed!\n"); _pCounter->notify_awaitable_completed(); });

		pthread.detach();
		//_pCounter->notify_awaitable_completed();
	}

	std::uint8_t m_commandData;

};

struct Promise
{
	auto initial_suspend()noexcept
	{
		printf("Initial suspend \n");
		return std::suspend_never{};
	}
	auto final_suspend()noexcept
	{
		printf("Final suspend \n");
		return std::suspend_never{};
	}

	void get_return_object()
	{
		printf("void get_return_object() \n");
	}
	void return_void()
	{
		printf("void return_void() \n");
	}

	void unhandled_exception()
	{
		while(1)
		{

		}
	}

};

TransmitTask sendCommand(std::uint8_t _sendCommand)
{
	return TransmitTask{ _sendCommand };
}



template <typename... Args>
struct std::coroutine_traits<void, Args ...>
{
	using promise_type = Promise;
};

//
//struct WhenAllTaskPromise
//{
//	using TCoroutineHandle = std::coroutine_handle<WhenAllTaskPromise>;
//	auto initial_suspend()
//	{
//		printf("auto initial_suspend()\n");
//		return std::suspend_always{};
//	}
//
//	auto final_suspend()
//	{
//		printf("auto final_suspend()\n");
//		return std::suspend_always{};
//	}
//
//	auto get_return_object()noexcept
//	{
//		printf("Construct return object auto get_return_object()noexcept\n");
//		return TCoroutineHandle::from_promise(*this);
//	}
//
//	void start(WhenAllCounter& counter) noexcept
//	{
//		m_counter = &counter;
//		auto coroutineHandle = TCoroutineHandle::from_promise(*this);
//		coroutineHandle.resume();
//	}
//
//	void return_void() const noexcept
//	{
//	}
//	WhenAllCounter* m_counter;
//};
//
//template< typename Awaitable>
//struct WhenAllTask
//{
//	using TPromise = WhenAllTaskPromise;
//	using TCoroutineHandle = std::coroutine_handle<WhenAllTaskPromise>;
//
//	using promise_type = TPromise;
//
//	WhenAllTask(TCoroutineHandle coroutine) noexcept
//		: m_suspendedCoroutine(coroutine)
//	{}
//
//	void launchTask(WhenAllCounter& _counter)
//	{
//		m_suspendedCoroutine.promise().start(_counter);
//	}
//
//	~WhenAllTask()
//	{
//		m_suspendedCoroutine.destroy();
//	}
//	TCoroutineHandle m_suspendedCoroutine;
//};
//
//template<typename Awaitable>
//WhenAllTask<void> makeWhenAllTask(Awaitable awaitable)
//{
//	co_await awaitable;
//}
//
//template<typename ... Tasks>
//struct WhenAllAwaitable
//{
//	explicit WhenAllAwaitable(Tasks&& ... tasks) noexcept
//		:m_counter{ sizeof...(Tasks) }
//		,m_taskList(std::move(tasks)...)
//	{
//	}
//	explicit WhenAllAwaitable(std::tuple<Tasks...>&& tasks)noexcept
//		: m_counter(sizeof...(Tasks))
//		, m_taskList(std::move(tasks))
//	{}
//
//	WhenAllCounter m_counter;
//	std::tuple<Tasks...> m_taskList;
//
//	bool is_ready()
//	{
//		return m_counter.is_ready();
//	}
//
//	auto operator co_await() noexcept
//	{
//		struct WhenAllAwaiter
//		{
//			WhenAllAwaiter(WhenAllAwaitable& awaitable) noexcept
//				: m_awaitable(awaitable)
//			{}
//
//			bool await_ready() const noexcept
//			{
//				printf("When all awaiter await ready\n");
//				return m_awaitable.is_ready();
//			}
//
//			bool await_suspend(std::coroutine_handle<> awaitingCoroutine) noexcept
//			{
//				printf("When all await suspend \n");
//				return m_awaitable.launch_tasks(awaitingCoroutine);
//			}
//
//			std::tuple<Tasks...>& await_resume() noexcept
//			{
//				printf("When all await resume\n");
//				return m_awaitable.m_taskList;
//			}
//
//		private:
//
//			WhenAllAwaitable& m_awaitable;
//
//		};
//		return WhenAllAwaiter{ *this };
//	}
//
//	void await_resume()
//	{
//		printf("Await Resume WhenAllAwaitable \n");
//	}
//
//	bool await_ready()
//	{
//		printf("Await Ready WhenAllAwaitable\n");
//		return false;
//	}
//
//	bool launch_tasks(std::coroutine_handle<> _coroHandle)noexcept
//	{
//		printf("launch_tasks WhenAllAwaitable\n");
//		std::apply(
//			[this](auto&... _task)->void
//			{
//				(_task.launchTask(m_counter), ...);
//			}
//			, m_taskList
//		);
//		return m_counter.try_await(_coroHandle);
//	}
//};
//
//
//template<typename ...Args>
//auto when_all(Args&& ... args)
//{
//	return WhenAllAwaitable{ std::make_tuple(makeWhenAllTask(args)...) };
//
//}
//


struct WhenAllSequence
{
	std::vector<TransmitTask> m_taskList;

	bool await_ready() const noexcept { return false; }

	void await_suspend(std::coroutine_handle<> handle)
	{
		for(auto& task : m_taskList)
		{
			co_await task;
		}
		handle.resume();
	}

	void await_resume()
	{
	}
};


template<typename ... Args>
auto when_all_sequence(Args&& ... args)
{
	std::vector<TransmitTask> m_taskList;
	(m_taskList.push_back(std::forward<Args&&>(args)),...);

	return WhenAllSequence{ std::move(m_taskList) };
}

void initializeProcedure()
{
	//co_await sendCommand(1);
	//co_await sendCommand(2);

	//co_await when_all(sendCommand(1), sendCommand(2), sendCommand(3));

	co_await when_all_sequence(sendCommand(1), sendCommand(2), sendCommand(3));
	co_await when_all_sequence(sendCommand(4), sendCommand(5), sendCommand(6));
	co_await when_all_sequence(sendCommand(7), sendCommand(8), sendCommand(9));
}

int main()
{

	initializeProcedure();

	while (true)
	{
		using namespace std::chrono_literals;
		printf("Inside main loop \n");
		std::this_thread::sleep_for(500ms);
	}
	return 0;
}
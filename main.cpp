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


template< typename Awaitable>
struct WhenAllTask
{
};

template<typename Awaitable>
WhenAllTask<void> makeWhenAllTask(Awaitable awaitable)
{
	co_await awaitable;
}

template<typename ... Tasks>
struct WhenAllAwaitable
{
	explicit WhenAllAwaitable(Tasks&& ... tasks) noexcept
		:m_counter{ sizeof...(Tasks) }
		,m_taskList(std::move(tasks)...)
	{
	}
	explicit WhenAllAwaitable(std::tuple<Tasks...>&& tasks)noexcept
		: m_counter(sizeof...(Tasks))
		, m_taskList(std::move(tasks))
	{}

	WhenAllCounter m_counter;
	std::tuple<Tasks...> m_taskList;

	bool is_ready()
	{
		return m_counter.is_ready();
	}

	auto operator co_await() noexcept
	{
		struct WhenAllAwaiter
		{
			WhenAllAwaiter(WhenAllAwaitable& awaitable) noexcept
				: m_awaitable(awaitable)
			{}

			bool await_ready() const noexcept
			{
				printf("When all awaiter await ready\n");
				return m_awaitable.is_ready();
			}

			bool await_suspend(std::coroutine_handle<> awaitingCoroutine) noexcept
			{
				printf("When all await suspend \n");
				return m_awaitable.launch_tasks(awaitingCoroutine);
			}

			std::tuple<Tasks...>& await_resume() noexcept
			{
				printf("When all await resume\n");
				return m_awaitable.m_taskList;
			}

		private:

			WhenAllAwaitable& m_awaitable;

		};
		return WhenAllAwaiter{ *this };
	}

	void await_resume()
	{
		printf("Await Resume WhenAllAwaitable \n");
	}

	bool await_ready()
	{
		printf("Await Ready WhenAllAwaitable\n");
		return false;
	}

	bool launch_tasks(std::coroutine_handle<> _coroHandle)noexcept
	{
		printf("launch_tasks WhenAllAwaitable\n");
		std::apply(
			[this](auto&... _task)
			{
				(_task.launch(&m_counter), ...);
			}
			, m_taskList
		);
		return m_counter.try_await(_coroHandle);
	}
};


template<typename ...Args>
auto when_all(Args&& ... args)
{
	return WhenAllAwaitable{ std::forward_as_tuple(args...) };

}

static std::array tasks = 
{
	TransmitTask(1),
	TransmitTask(2),
	TransmitTask(3),
	TransmitTask(4)
};

void initializeProcedure()
{
	//co_await sendCommand(1);
	//co_await sendCommand(2);

	co_await when_all(sendCommand(1), sendCommand(2), sendCommand(3));

	for (auto&& task : tasks) {
		co_await task;
	}
}
//
//cppcoro::task<> transmitData()
//{
//	printf("Transmit Data\n");
//}
//
//void initializeDisplay()
//{
//	co_await cppcoro::when_all(transmitData(), transmitData(), transmitData());
//}

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
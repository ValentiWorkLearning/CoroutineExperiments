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
		std::uint8_t transmitValue = m_commandData;
		auto pthread = std::thread([_coroHandle, transmitValue] { std::this_thread::sleep_for(2100ms); printf("Task completed!,Tansmitted %d\n", static_cast<std::uint16_t>(transmitValue)); _coroHandle.resume(); });
		pthread.detach();
		
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

struct VoidTask
{
	struct task_promise;
	using promise_type = task_promise;

	struct task_promise
	{
		task_promise()noexcept = default;
		void return_void()noexcept {}
		void unhandled_exception() noexcept
		{
			while(true)
			{

			}
		}
		VoidTask get_return_object()noexcept
		{
			return VoidTask{ std::coroutine_handle<task_promise>::from_promise(*this) };
		}

		auto initial_suspend()noexcept { return std::suspend_always{}; }

		struct final_awaitable
		{
			bool await_ready()
			{
				return false;
			}
			template<typename TPromise>
			void await_suspend(std::coroutine_handle<TPromise> coroutine)
			{
				task_promise& promise = coroutine.promise();
				if (promise.m_state.exchange(true))
				{
					promise.m_continuation.resume();
				}
			}

			void await_resume()
			{
			}
		};

		bool try_set_continuation(std::coroutine_handle<> continuation)
		{
			m_continuation = continuation;
			return m_state = true;
		}

		auto final_suspend()
		{
			return final_awaitable{};
		}

		std::coroutine_handle<> m_continuation;
		std::atomic_bool m_state{false};
	};
	
	struct task_awaitable
	{
		task_awaitable(std::coroutine_handle<promise_type> coroutine)
			: m_coroutine{coroutine}
		{
		}
		bool await_ready()const noexcept
		{
			return !m_coroutine || m_coroutine.done();
		}
		bool await_suspend( std::coroutine_handle<> awaitingRoutine )
		{
			m_coroutine.resume();
			return m_coroutine.promise().try_set_continuation(awaitingRoutine);
		}

		void await_resume()
		{
		}

		std::coroutine_handle<promise_type> m_coroutine;
	};

	VoidTask(std::coroutine_handle<task_promise> suspendedCoroutine)
		:	m_coroutine{suspendedCoroutine}
	{
	}

	~VoidTask()
	{
		if(m_coroutine)
			m_coroutine.destroy();
	}

	bool await_ready() const noexcept
	{
		return !m_coroutine||m_coroutine.done();
	}

	auto operator co_await() noexcept
	{
		return task_awaitable{ m_coroutine };
	}

	void await_resume()
	{
	}

	std::coroutine_handle<promise_type> m_coroutine;
};

template<typename ... Tasks>
struct WhenAllSequence
{
	std::tuple<Tasks...> m_taskList;

	explicit WhenAllSequence(Tasks&& ... tasks) noexcept
		:	m_taskList(std::move(tasks)...)
	{
	}

	explicit WhenAllSequence(std::tuple<Tasks...>&& tasks)noexcept
		:	m_taskList(std::move(tasks))
	{
	}

	bool await_ready() const noexcept { return false; }

	void await_suspend(std::coroutine_handle<> handle)
	{
		co_await std::apply(
				[](auto ... task)->VoidTask
				{
					(co_await task, ...);

					printf("std::apply! completed\n");
				}
			,	m_taskList
		);
		handle.resume();
		printf("Requested coroytine resume\n");
	}

	void await_resume()
	{
	}
};


template<typename ... Args>
auto when_all_sequence(Args&& ... args)
{

	return WhenAllSequence{ std::make_tuple(std::move(args)...) };
}

auto sequenceTransmit(int forTest)
{
	return when_all_sequence(sendCommand(11 + forTest), sendCommand(12 + forTest));
}

void initializeProcedure()
{
	co_await when_all_sequence(sendCommand(1), sendCommand(2), sendCommand(3));
	co_await when_all_sequence(sendCommand(4), sendCommand(5), sendCommand(6));
	co_await when_all_sequence(sendCommand(7), sendCommand(8), sendCommand(9));
	co_await when_all_sequence(sequenceTransmit(1), sequenceTransmit(2));
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
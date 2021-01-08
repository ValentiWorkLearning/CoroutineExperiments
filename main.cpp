#include <coroutine>
#include <cstdint>
#include <chrono>
#include <thread>
#include <atomic>
#include <cstdio>
#include <tuple>
#include <vector>
#include <array>

#ifdef _MSC_VER
#include <coroutine>
namespace stdcoro = std;
#elif __GNUC__
#include <coroutine>
namespace stdcoro = std;
#else
#include <experimental/coroutine>
namespace stdcoro = std::experimental
#endif

struct Promise;
struct TransmitTask
{
	explicit constexpr TransmitTask(std::uint8_t command)
		:m_commandData{ command }
	{
	}

	void await_resume() const
	{
		printf("Await Resume Transmit \n");
	}

	bool await_ready() const noexcept
	{
		printf("Await Ready Transmit\n");
		return false;
	}

	void await_suspend(stdcoro::coroutine_handle<> _coroHandle)
	{
		printf("Await suspend Transmit\n");
		using namespace std::chrono_literals;
		std::uint8_t transmitValue = m_commandData;
		auto pthread = std::thread([_coroHandle, transmitValue]()mutable { std::this_thread::sleep_for(2100ms); printf("Task completed!,Tansmitted %d\n", static_cast<std::uint16_t>(transmitValue)); _coroHandle.resume(); });
		pthread.detach();

	}

	std::uint8_t m_commandData;

};

struct Promise
{
	auto initial_suspend()noexcept
	{
		printf("Initial suspend \n");
		return stdcoro::suspend_never{};
	}
	auto final_suspend()noexcept
	{
		printf("Final suspend \n");
		return stdcoro::suspend_never{};
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
		while (1)
		{

		}
	}

};

TransmitTask sendCommand(std::uint8_t _sendCommand)
{
	return TransmitTask{ _sendCommand };
}



template <typename... Args>
struct stdcoro::coroutine_traits<void, Args ...>
{
	using promise_type = Promise;
};

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
			while (true)
			{

			}
		}
		VoidTask get_return_object()noexcept
		{
			return VoidTask{ stdcoro::coroutine_handle<task_promise>::from_promise(*this) };
		}

		auto initial_suspend()noexcept { return stdcoro::suspend_always{}; }

		struct final_awaitable
		{
			bool await_ready()
			{
				return false;
			}
			template<typename TPromise>
			void await_suspend(stdcoro::coroutine_handle<TPromise> coroutine)
			{
				task_promise& promise = coroutine.promise();
				if (promise.m_continuation)
				{
					promise.m_continuation.resume();
				}
			}

			void await_resume()noexcept {}
		};

		void set_continuation(stdcoro::coroutine_handle<> continuation)
		{
			m_continuation = continuation;
		}

		auto final_suspend()noexcept
		{
			return final_awaitable{};
		}

		stdcoro::coroutine_handle<> m_continuation;
	};

	struct task_awaitable
	{
		task_awaitable(stdcoro::coroutine_handle<promise_type> coroutine)
			: m_coroutine{ coroutine }
		{
		}
		bool await_ready()const noexcept
		{
			return !m_coroutine || m_coroutine.done();
		}
		void await_suspend(stdcoro::coroutine_handle<> awaitingRoutine)
		{
			m_coroutine.resume();
			m_coroutine.promise().set_continuation(awaitingRoutine);
		}

		void await_resume()
		{
		}

		stdcoro::coroutine_handle<promise_type> m_coroutine;
	};

	VoidTask(stdcoro::coroutine_handle<task_promise> suspendedCoroutine)
		: m_coroutine{ suspendedCoroutine }
	{
	}

	~VoidTask()
	{
		if (m_coroutine)
			m_coroutine.destroy();
	}

	bool await_ready() const noexcept
	{
		return !m_coroutine || m_coroutine.done();
	}

	auto operator co_await() noexcept
	{
		return task_awaitable{ m_coroutine };
	}

	void await_resume()
	{
	}

	stdcoro::coroutine_handle<promise_type> m_coroutine;
};

template<typename ... Tasks>
struct WhenAllSequence
{
	std::tuple<Tasks...> m_taskList;

	explicit WhenAllSequence(Tasks&& ... tasks) noexcept
		: m_taskList(std::move(tasks)...)
	{
	}

	explicit WhenAllSequence(std::tuple<Tasks...>&& tasks)noexcept
		: m_taskList(std::move(tasks))
	{
	}

	bool await_ready() const noexcept { return false; }

	void await_suspend(stdcoro::coroutine_handle<> handle) noexcept
	{
		co_await launchAll(std::make_integer_sequence<std::size_t, sizeof...(Tasks)>{});
		handle.resume();
		printf("Requested coroutine resume\n");
	}

	template<std::size_t... Indexes>
	VoidTask launchAll(std::integer_sequence<std::size_t, Indexes...>)
	{
		(void)std::initializer_list<int>{
			(co_await std::get<Indexes>(m_taskList), 0)...
		};
	}

	void await_resume() noexcept
	{
	}
};


template<typename ... Args>
auto when_all_sequence(Args&& ... args) noexcept
{
	return WhenAllSequence{ std::make_tuple(std::move(args)...) };
}

auto sequenceTransmit(int forTest)noexcept
{
	return when_all_sequence(sendCommand(11 + forTest), sendCommand(12 + forTest));
}

void initializeProcedure() noexcept
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

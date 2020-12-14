#include <coroutine>
#include <cstdint>
#include <chrono>
#include <thread>

struct Promise;
struct TransmitTask
{
	explicit TransmitTask(std::uint8_t command)
		:m_commandData{command}
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

	void await_suspend( std::coroutine_handle<> _coroHandle)
	{
		printf("Await suspend Transmit\n");
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(100ms);
		_coroHandle.resume();
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

};

TransmitTask sendCommand( std::uint8_t _sendCommand)
{
	return TransmitTask{ _sendCommand };
}

template <typename... Args>
struct std::coroutine_traits<void, Args ...>
{
	using promise_type = Promise;
};


template<typename ... Tasks>
struct WhenAllTask
{
	std::tuple<Tasks...> m_taskList;

	void await_resume()
	{
		printf("Await Resume WhenAllTask \n");
	}

	bool await_ready()
	{
		printf("Await Ready WhenAllTask\n");
		return false;
	}

	void await_suspend(std::coroutine_handle<> _coroHandle)
	{
		printf("Await Suspend WhenAllTask\n");
		std::apply(
			[](auto&... _task)
			{
				auto applyTask = [](auto&& task)
				{
					printf("Called from std::apply\n");
				};
				(applyTask(_task), ...);

			}
			, m_taskList
		);
	}
};


template<typename ...Args>
auto when_all(Args&& ... args)
{
	return WhenAllTask{ std::forward_as_tuple(args...) };

}

void initializeProcedure()
{
	//co_await sendCommand(1);
	//co_await sendCommand(2);

	// wanted: co_await when_all( sendCommand(1),sendCommand(2) );

	co_await when_all(sendCommand(1), sendCommand(2));
}


int main()
{

	initializeProcedure();

	while (true)
	{
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(100ms);
	}
	return 0;
}
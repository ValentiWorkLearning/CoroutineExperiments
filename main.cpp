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
		printf("Await Resume \n");
	}

	bool await_ready()
	{
		printf("Await Ready \n");
		return false;
	}

	void await_suspend( std::coroutine_handle<> _coroHandle)
	{
		printf("Await suspend \n");
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

void initializeProcedure()
{
	co_await sendCommand(1);
	co_await sendCommand(2);
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
#include <iostream>
#include <experimental/coroutine>

#include <array>
#include <chrono>
#include <thread>
#include <cstdint>

// https://blog.panicsoftware.com/your-first-coroutine/
// https://manybutfinite.com/post/anatomy-of-a-program-in-memory/
// https://manybutfinite.com/post/journey-to-the-stack/
// https://luncliff.github.io/posts/Exploring-MSVC-Coroutine.html
// https://habr.com/en/post/493808/ // coroutine code linearizing
// https://lewissbaker.github.io/2017/09/25/coroutine-theory
// https://lewissbaker.github.io/2018/09/05/understanding-the-promise-type
// https://devblogs.microsoft.com/oldnewthing/20191219-00/?p=103230
// https://habr.com/en/post/348602/ // boost asio with coroutines TS
// https://m.habr.com/ru/post/519464/ // C++20 coroutines
// https://mariusbancila.ro/blog/2020/06/22/a-cpp20-coroutine-example/

// We have to define the 'resumable' class for the way
// of communicating between the coroutine and usual function
class resumable
{
public:

    // Here we declare the promise type with necessary functions 
    struct promise_type
    {
        // //declaration of the coroutine handle alias- basic type for operating with coroutine;
        using coro_handle = std::experimental::coroutine_handle<promise_type>;

        resumable get_return_object()
        {
            return coro_handle::from_promise(*this);
        }
        auto initial_suspend()
        {
            return std::experimental::suspend_always();
        }
        auto final_suspend()noexcept
        {
            return std::experimental::suspend_always();
        }

        void unhandled_exception()
        {
            std::terminate();
        }

        void return_void() {}
    };

    //declaration of the coroutine handle alias- basic type for operating with coroutine;
    using coro_handle = std::experimental::coroutine_handle<promise_type>;

public:

    resumable(coro_handle handle)
        :   m_coroutineHandle{handle}
    {
    }

    resumable(resumable&) = delete;
    resumable(resumable&&) = delete;

    bool resume()
    {
        if( !m_coroutineHandle.done() )
            m_coroutineHandle.resume();

        return !m_coroutineHandle.done();
    }
private:
    coro_handle m_coroutineHandle;
};

resumable foo()
{
    std::cout<<"Hello"<<std::endl;
    // Without resumable return type we cant't use our coroutine
    // or without overriding the co_await operator also

    co_await std::experimental::suspend_always();
    std::cout<<"from coroutine";
}


void spiTrasnmitBuffer(
        std::uint8_t* _pBuffer
    ,   std::uint16_t _bufferSize
)
{
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(100ms);
}

auto commandBufferFirst = std::array{ 0x00u, 0x01u, 0x02u, 0x03u };
auto commandBufferSecond = std::array{ 0x04u, 0x05u, 0x06u, 0x07u,0x08u };

void initDisplay()
{
    co_await spiTrasnmitBuffer(commandBufferFirst.data(), commandBufferFirst.size());
    co_await spiTrasnmitBuffer(commandBufferSecond.data(), commandBufferSecond.size());
}


//For using co_await operator we have to use the concept of 'awaitable entity'
//the main idea is quite simple - to provide the implementation for the customization_point of the coroutine
int main()
{
    auto resumableObject = foo();
    resumableObject.resume();
    resumableObject.resume();

    return 0;
}
#include <iostream>
#include <experimental/coroutine>

// https://blog.panicsoftware.com/your-first-coroutine/

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

//For using co_await operator we have to use the concept of 'awaitable entity'
//the main idea is quite simple - to provide the implementation for the customization_point of the coroutine
int main()
{
    auto resumableObject = foo();
    resumableObject.resume();
    resumableObject.resume();
    return 0;
}
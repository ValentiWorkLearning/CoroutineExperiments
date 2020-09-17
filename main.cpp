#include <iostream>
#include <experimental/coroutine>

// https://blog.panicsoftware.com/your-first-coroutine/

// We have to define the 'resumable' class for the way
// of communicating between the coroutine and usual function
class resumable
{
public:
    bool resume()
    {

    }
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
    foo();
    return 0;
}
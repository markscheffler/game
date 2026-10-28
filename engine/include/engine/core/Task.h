#pragma once

#include <coroutine>
#include <functional>
#include <thread>
#include <utility>
#include <print>
#include <tuple>
#include <type_traits>

template<class F, class... Args>
struct PerformTask
{
    F function;
    std::tuple<Args...> params;

    PerformTask(F f, Args... args)
        : function(std::forward<F>(f)),
        params(std::make_tuple(std::forward<Args...>(args)...))
    {}

    using ReturnType = std::invoke_result_t<F, Args...>();
    //using Type = std::conditional_t<std::is_void_v<ReturnType>, void, ReturnType>;
    
    ReturnType value;

        auto await_ready() noexcept { return false; }
        auto await_suspend(std::coroutine_handle<> handle) noexcept
        {
            if constexpr (std::is_void_v<ReturnType>)
            {
                std::thread([handle, *this]() 
                { 
                    std::apply(function, params);
                }
            
            ).detach();

            }
            else
            {
                std::thread([handle, *this]()
                    {
                       std::apply(function, params);
                    }
                ).detach();
            }
        }

        auto await_resume() noexcept {}
};

template<typename F>
struct BuildSciptsAwaitable 
{
    F func_;

    BuildSciptsAwaitable(F f)
        : func_(std::forward<F>(f))
    {
    }
    
    auto await_ready() noexcept { return false; }
    auto await_suspend(std::coroutine_handle<> handle) noexcept
    {
        std::thread([handle, *this]() { 
            std::println("creating thread id: {} ", std::this_thread::get_id());
            func_();
        }).detach();
    }
    void await_resume() noexcept
    {
    }
};



template <typename T> 
struct Coroutine 
{
    
    struct promise_type {
        T value;
        Coroutine get_return_object() {
            return Coroutine{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_never initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void unhandled_exception() {}
        template <std::convertible_to<T> From> std::suspend_always yield_value(From&& f)
        {
            value = std::forward<From>(f);
            return {};
        }

        void return_value(T v) {
            value = std::move(v);
            return {};
        }
        
    };
    std::coroutine_handle<promise_type> h{};

 

    ~Coroutine() {
        if (h)
            h.destroy();
    }
};
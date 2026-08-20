#pragma once


#include <stdexcept>
#include <string>
#include <stacktrace>
#include <format>

namespace fire
{
    class Exception: public std::runtime_error
    {
        public:

        template<class... Args>
        constexpr Exception(std::format_string<Args...> msg, Args &&... args)
        :std::runtime_error{std::format(msg, std::forward<Args>(args)...)}
        ,m_what{std::format("{} \n {}", std::runtime_error::what(),
            std::stacktrace::current(1)).c_str()}
        {

        }

        constexpr std::string ToString(this auto&& self)
        {
            return self.m_what;
        }
        
        constexpr const char* what()const override
        {
            return m_what.c_str();
        }

        private:
        std::string m_what;
    };

}
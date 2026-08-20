#pragma once

#include <print>
#include <format>
#include <source_location>
#include <fstream>

// inline static auto log_file = []
// {
//     auto f = std::ofstream{"log.txt", std::ios::app};
//     if (!f)
//     {
//         std::terminate();
//     }

//     return f;
// }();

namespace fire::log
{
	enum class level{trace, warn, debug, fatal};


	template<level L, typename... Args>
	struct Print
	{
		Print(std::format_string<Args...>msg, Args&& ...args, const std::source_location loc = std::source_location::current())
		{

			std::string l = "";

			if constexpr (L == level::debug)
			{
				l = "DEBUG";
			}
			else if constexpr (L == level::trace)
			{
				l = "TRACE";
			}
			else if constexpr(L == level::warn)
			{
				l = "WARN";
			}
			else if constexpr (L == level::fatal)
			{
				l = "FATAL";
			}

		auto log_line = std::format("[{}] {} : line - {} {}", l, loc.file_name(), loc.line(), 
            std::format(msg, std::forward<Args>(args)...));

			//log_file << log_line << '\n';

            std::println("{}", log_line);
		}
	};

	template<level L, typename... Args >
	Print(std::format_string<Args...>, Args&&...) -> Print < L, Args...>;

	template<typename... Args>
	using trace = Print<level::trace, Args... >;

	template<typename... Args>
	using warn = Print<level::warn, Args...>;

	template<typename... Args>
	using debug = Print<level::debug, Args...>;

	template<typename... Args>
	using fatal = Print<level::fatal, Args...>;


#if defined(NDEBUG)

template<typename... Args>
using trace = Print<level::trace, Args...> {};

template<typename... Args>
using warn = Print<level::warn, Args...>{};

template<typename... Args>
using debug = Print<level::debug, Args>{};

template<typename... Args>
using fatal = Print<level::fatal, Args...> {}

#endif
}
#pragma once

#include "Exception.h"


namespace fire
{
    template<typename... Args>
    constexpr void verify(bool predicate, std::format_string<Args...>(msg), Args&&... args)
    {
        if(!predicate)
        throw fire::Exception(msg, std::forward<Args>(args)...);
    }
}
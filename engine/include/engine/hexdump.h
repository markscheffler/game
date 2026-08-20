#pragma once
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <cstdint>
#include <type_traits>
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <vector>

namespace fire
{
    inline std::string hex_dump(std::span<const std::byte> bytes,
    std::size_t bytes_per_line = 16)
    {
        static constexpr char lut[] = "0123456789abcdef";

        std::string out;
        out.reserve(bytes.size() * 4);
        std::size_t offset = 0;

        for(auto line : bytes | std::views::chunk(bytes_per_line))
        {
            {
                char buf[9];
                std::snprintf(buf, sizeof(buf), "%08zx", offset);
                out.append(buf);
            }
            
            out.append(" ");
            for(std::byte b: line)
            {
                std::uint8_t v = std::to_integer<std::uint8_t>(b);
                out.push_back(lut[v >> 4]);
                out.push_back(lut[v & 0xf]);
                out.push_back(' ');
            }

            if(line.size() < bytes_per_line)
            {
                out.append((bytes_per_line - line.size()) * 3, ' ');
            }

            out.append(" |");

            for(std::byte b: line)
            {
                std::uint8_t v = std::to_integer<std::uint8_t>(b);
                out.push_back(std::isprint(v) ? char(v) : '.');

            }
            out.append("|\n");
            offset +=line.size();
        }
        return out;
    }

    // inline std::string hex_dump(std::string_view s)
    // {
    //     //return hex_dump(std::as_bytes(std::span{s}));

    //     std::stringstream ss;

    //     auto currentflags = ss.flags();
    //     for(auto elem : s)
    //     {
    //         ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<std::uint8_t>(elem) << ' ';
    //     }
    //     ss.flags(currentflags);

    //     if(ss.str().ends_with(' '))
    //     {
    //         ss.str().pop_back();
    //     }
    //     return ss.str();

    // }

    inline std::string hex_dump(const std::vector<std::uint8_t>&v, std::size_t bpl = 16)
    {
        return hex_dump(std::as_bytes(std::span{v}), bpl);
    }

    inline std::string hex_dump(const std::string& s, std::size_t bpl = 16)
    {
        return hex_dump(std::as_bytes(std::span{s}), bpl);
    }

    template<std::size_t N>
    inline std::string hex_dump(const std::array<std::uint8_t, N>& a, std::size_t bpl = 16)
    {
        return hex_dump(std::as_bytes(std::span{a}), bpl);
    }

} 

#pragma once

#include "space_size.h" // IWYU pragma: export

#include <filesystem>

namespace stdsharpinline ::literals
{
    inline std::filesystem::path operator""_path(const char* const str, const std::size_t len)
    {
        return {str, str + len};
    }

    inline std::filesystem::path
        operator""_native_path(const char* const str, const std::size_t len)
    {
        return {str, str + len, std::filesystem::path::format::native_format};
    }

    inline std::filesystem::path
        operator""_generic_path(const char* const str, const std::size_t len)
    {
        return {str, str + len, std::filesystem::path::format::generic_format};
    }
}

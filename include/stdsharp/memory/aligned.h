#pragma once

#include "../cstdint/cstdint.h"
#include "pointer_traits.h"

#include <span>

namespace stdsharp
{
    inline constexpr struct align_fn
    {
        template<non_const T, std::size_t Size>
        auto operator()(
            const std::size_t alignment,
            const std::size_t size,
            const std::span<T, Size>& span
        ) const noexcept
        {
            auto space = span.size_bytes();
            void* void_ptr = span.data();
            std::align(alignment, size, void_ptr, space);
            return void_ptr == nullptr ? //
                std::span<T>{} :
                std::span{pointer_cast<T>(void_ptr), space / sizeof(T)};
        }
    } align{};

    inline constexpr struct is_align_fn
    {
        bool operator()(const std::size_t alignment, const std::size_t size, void* const ptr)
            const noexcept
        {
            return (*this)(alignment, size, ptr, size);
        }

        bool operator()(
            const std::size_t alignment,
            const std::size_t size,
            void* const ptr,
            const std::size_t bytes
        ) const noexcept
        {
            return align(alignment, size, std::span{pointer_cast<byte>(ptr), bytes}).data() == ptr;
        }
    } is_align{};
}

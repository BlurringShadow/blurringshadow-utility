#pragma once

#include "../cassert/cassert.h"
#include "../iterator/basic_iterator.h"

#include <new>

#include "../compilation_config_in.h"

namespace stdsharp::details
{
    template<typename T>
    class launder_iterator
    {
    };

}

namespace stdsharp
{
    template<typename T>
    struct STDSHARP_EBO launder_iterator :
        basic_iterator<T, std::ptrdiff_t, std::contiguous_iterator_tag>
    {
        T* ptr_;

    public:
        using iterator_category = std::contiguous_iterator_tag;

        launder_iterator() = default;

        constexpr launder_iterator(T* const ptr) noexcept: ptr_(ptr) {}

        [[nodiscard]] constexpr T* data() const noexcept { return std::launder(ptr_); }

        [[nodiscard]] constexpr T& operator*() const noexcept
        {
            assert_not_null(data());
            return *data();
        }

        constexpr auto& operator++() noexcept
        {
            ++ptr_;
            return *this;
        }

        constexpr auto& operator--() noexcept
        {
            --ptr_;
            return *this;
        }

        constexpr auto& operator+=(const std::ptrdiff_t diff) noexcept
        {
            ptr_ += diff;
            return *this;
        }

        [[nodiscard]] constexpr auto operator-(const launder_iterator iter) const noexcept
        {
            return data() - iter.data();
        }

        [[nodiscard]] constexpr decltype(auto) operator[](const std::ptrdiff_t diff) const noexcept
        {
            assert_not_null(data());
            return *std::launder(data() + diff);
        }

        [[nodiscard]] constexpr auto operator<=>(const launder_iterator& other) const noexcept
        {
            return data() <=> other.data();
        }

        [[nodiscard]] constexpr bool operator==(const launder_iterator& other) const noexcept
        {
            return data() == other.data();
        }
    };

    template<typename T>
    launder_iterator(T*) -> launder_iterator<T>;

    template<typename T>
    struct launder_const_iterator : launder_iterator<const T>
    {
        using launder_iterator<const T>::launder_iterator;
    };

    template<typename T>
    launder_const_iterator(T*) -> launder_const_iterator<T>;
}

#include "../compilation_config_out.h"

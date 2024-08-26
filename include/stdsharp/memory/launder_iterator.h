#pragma once

#include "../cassert/cassert.h"
#include "../iterator/basic_iterator.h"
#include "../utility/fwd_cast.h"

#include <new>

#include "../compilation_config_in.h"

namespace stdsharp
{
    template<typename T>
    class STDSHARP_EBO launder_iterator :
        public basic_iterator<T, std::ptrdiff_t, std::contiguous_iterator_tag>
    {
        using m_base = basic_iterator<T, std::ptrdiff_t, std::contiguous_iterator_tag>;

        T* ptr_;

        static constexpr auto self_cast = fwd_cast<launder_iterator>;

    public:
        using m_base::operator++;
        using m_base::operator--;
        using m_base::operator*;
        using m_base::operator-;

        launder_iterator() = default;

        constexpr launder_iterator(T* const ptr) noexcept: ptr_(ptr) {}

        [[nodiscard]] constexpr T* data() const noexcept { return std::launder(ptr_); }

        [[nodiscard]] constexpr T& operator*() const noexcept
        {
            assert_not_null(data());
            return *data();
        }

        constexpr auto& operator++(this non_const auto& self) noexcept
        {
            ++self_cast(self).ptr_;
            return self;
        }

        constexpr auto& operator--(this non_const auto& self) noexcept
        {
            --self_cast(self).ptr_;
            return self;
        }

        constexpr auto& operator+=(this non_const auto& self, const std::ptrdiff_t diff) noexcept
        {
            self_cast(self).ptr_ += diff;
            return self;
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

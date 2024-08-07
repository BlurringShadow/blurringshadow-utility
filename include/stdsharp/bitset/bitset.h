#pragma once

#include "../cassert/cassert.h"
#include "../cstdint/cstdint.h"
#include "../iterator/basic_iterator.h"

#include <bitset>
#include <ranges>

#include "../compilation_config_in.h"

namespace stdsharp::details
{
    template<std::size_t N, bool IsConst>
    class bitset_iterator
    {
    public:
        using bitset = std::bitset<N>;

    protected:
        using maybe_const = std::conditional_t<IsConst, const bitset, bitset>;

        maybe_const* set_{};
        std::size_t i_{};

    public:
        bitset_iterator() = default;

        constexpr bitset_iterator(maybe_const& set, const std::size_t index): set_(&set), i_(index)
        {
        }

        [[nodiscard]] constexpr decltype(auto) operator*() const { return (*set_)[i_]; }

        template<typename T>
        constexpr auto& operator++(this T& self) noexcept
        {
            ++(forward_cast<T&, bitset_iterator>(self).i_);
            return self;
        }

        template<typename T>
        constexpr auto& operator--(this T& self) noexcept
        {
            --(forward_cast<T&, bitset_iterator>(self).i_);
            return self;
        }

        template<typename T>
        constexpr auto& operator+=(this T& self, const ssize_t n) noexcept
        {
            forward_cast<T&, bitset_iterator>(self).i_ += n;
            return self;
        }

        [[nodiscard]] constexpr auto operator-(const bitset_iterator& other) const noexcept
        {
            assert_equal(this->set_, other.set_);
            return static_cast<ssize_t>(i_) - static_cast<ssize_t>(other.i_);
        }

        [[nodiscard]] constexpr decltype(auto) operator[](const ssize_t index) const
        {
            return (*this->set_)[this->i_ + index];
        }

        [[nodiscard]] constexpr bool operator==(const bitset_iterator& other) const noexcept
        {
            assert_equal(this->set_, other.set_);
            return this->i_ == other.i_;
        }

        [[nodiscard]] constexpr auto operator<=>(const bitset_iterator& other) const noexcept
        {
            assert_equal(this->set_, other.set_);
            return this->i_ <=> other.i_;
        }
    };

}

namespace stdsharp
{
    template<std::size_t N>
    struct bitset_iterator : basic_iterator<details::bitset_iterator<N, false>>
    {
        using basic_iterator<details::bitset_iterator<N, false>>::basic_iterator;
    };

    template<std::size_t N>
    bitset_iterator(std::bitset<N>&, auto) -> bitset_iterator<N>;

    template<std::size_t N>
    struct bitset_const_iterator : basic_iterator<details::bitset_iterator<N, true>>
    {
        using basic_iterator<details::bitset_iterator<N, true>>::basic_iterator;
    };

    template<std::size_t N>
    bitset_const_iterator(const std::bitset<N>&, auto) -> bitset_const_iterator<N>;

    inline constexpr struct bitset_crng_fn
    {
        template<std::size_t N>
        [[nodiscard]] constexpr auto operator()(const std::bitset<N>& set) const
        {
            return std::ranges::
                subrange{bitset_const_iterator{set, 0}, bitset_const_iterator{set, N}};
        }
    } bitset_crng{};

    inline constexpr struct bitset_rng_fn : bitset_crng_fn
    {
        using bitset_crng_fn::operator();

        template<std::size_t N>
        [[nodiscard]] constexpr auto operator()(std::bitset<N>& set) const
        {
            return std::ranges::subrange{bitset_iterator{set, 0}, bitset_iterator{set, N}};
        }
    } bitset_rng{};
}

#include "../compilation_config_out.h"

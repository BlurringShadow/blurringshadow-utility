#pragma once

#include "../ranges/ranges.h"
#include "allocator_traits.h"

#include "../compilation_config_in.h"

namespace stdsharp::details
{
    template<typename Alloc>
    struct allocation_traits
    {
        using pointer = allocator_pointer<Alloc>;
        using const_pointer = allocator_const_pointer<Alloc>;
        using size_type = allocator_size_type<Alloc>;

        using callocation_base =
            std::ranges::subrange<const_pointer, const_pointer, std::ranges::subrange_kind::sized>;

        using allocation_base =
            std::ranges::subrange<pointer, const_pointer, std::ranges::subrange_kind::sized>;

        struct basic_allocation
        {
            using allocator_type = Alloc;
        };

        struct STDSHARP_EBO callocation : callocation_base, basic_allocation
        {
            using callocation_base::callocation_base;

            callocation() = default;

            constexpr callocation(const const_pointer p, const size_type diff) noexcept:
                callocation_base(p, p + diff)
            {
            }
        };

        struct allocation : allocation_base, basic_allocation
        {
            using allocation_base::allocation_base;

            allocation() = default;

            constexpr allocation(const pointer p, const size_type diff) noexcept:
                allocation_base(p, p + diff)
            {
            }

            constexpr auto& operator=(const auto& t) noexcept
                requires std::constructible_from<
                    allocation_base,
                    std::ranges::iterator_t<decltype(t)>,
                    std::ranges::sentinel_t<decltype(t)>>
            {
                static_cast<allocation_base&>(*this) =
                    allocation_base{std::ranges::begin(t), std::ranges::end(t)};
                return *this;
            }

            constexpr operator callocation() const noexcept
            {
                return callocation{allocation_base::begin(), allocation_base::end()};
            }
        };
    };
}

namespace stdsharp
{
    template<allocator_req Alloc>
    using callocation = details::allocation_traits<Alloc>::callocation;

    template<allocator_req Alloc>
    using allocation = details::allocation_traits<Alloc>::allocation;

    template<allocator_req Alloc>
    inline constexpr allocation<Alloc> empty_allocation{};

    template<allocator_req Alloc, typename T = Alloc::value_type>
    struct allocation_cdata_fn
    {
        constexpr decltype(auto) operator()(const callocation<Alloc>& allocation) const noexcept
        {
            return pointer_cast<T>(std::ranges::begin(allocation));
        }
    };

    template<allocator_req Alloc, typename T = Alloc::value_type>
    inline constexpr allocation_cdata_fn<Alloc, T> allocation_cdata{};

    template<allocator_req Alloc, typename T = Alloc::value_type>
    struct allocation_data_fn
    {
        constexpr decltype(auto) operator()(const allocation<Alloc>& allocation) const noexcept
        {
            return pointer_cast<T>(std::ranges::begin(allocation));
        }

        constexpr decltype(auto) operator()(const callocation<Alloc>& allocation) const noexcept
        {
            return pointer_cast<T>(std::ranges::begin(allocation));
        }
    };

    template<allocator_req Alloc, typename T = Alloc::value_type>
    inline constexpr allocation_data_fn<Alloc, T> allocation_data{};

    template<allocator_req Alloc, typename T = Alloc::value_type>
    struct allocation_get_fn
    {
        template<typename U>
        constexpr decltype(auto) operator()(const U& rng) const
            noexcept(noexcept(*allocation_data<Alloc, T>(rng)))
            requires std::invocable<allocation_data_fn<Alloc, T>, const U&>
        {
            return *std::launder(allocation_data<Alloc, T>(rng));
        }
    };

    template<allocator_req Alloc, typename T = Alloc::value_type>
    inline constexpr allocation_get_fn<Alloc, T> allocation_get{};

    template<allocator_req Alloc, typename T = Alloc::value_type>
    using allocation_cget_fn = allocation_get_fn<Alloc, const T>;

    template<allocator_req Alloc, typename T = Alloc::value_type>
    inline constexpr allocation_cget_fn<Alloc, T> allocation_cget{};

    template<typename Rng, typename Alloc>
    concept callocations = std::ranges::input_range<Rng> &&
        nothrow_convertible_to<range_const_reference_t<Rng>, const callocation<Alloc>&>;

    template<typename Rng, typename Alloc>
    concept allocations = requires(const allocation<Alloc>& alloction) {
        requires callocations<Rng, Alloc>;
        requires std::ranges::output_range<Rng, decltype(alloction)>;
        requires nothrow_convertible_to<range_const_reference_t<Rng>, decltype(alloction)>;
    };
}

#include "../compilation_config_out.h"

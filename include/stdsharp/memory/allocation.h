#pragma once

#include "../ranges/ranges.h"
#include "allocator_traits.h"

#include "../compilation_config_in.h"

namespace stdsharp::details
{
    template<typename Alloc>
    struct allocation_traits
    {
        using const_pointer = allocator_const_pointer<Alloc>;

        template<
            typename Ptr = const_pointer,
            typename Base =
                std::ranges::subrange<Ptr, const_pointer, std::ranges::subrange_kind::sized>>
        struct basic_allocation : Base
        {
            using Base::Base;

            using allocator_type = Alloc;

            basic_allocation() = default;

            constexpr basic_allocation(const Ptr p, const allocator_size_type<Alloc>& diff) noexcept
                :
                Base(p, p + diff)
            {
            }
        };

        struct callocation : basic_allocation<>
        {
        };

        struct allocation : basic_allocation<allocator_pointer<Alloc>>
        {
            constexpr operator callocation() const noexcept
            {
                return callocation{this->begin(), this->end()};
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

}

namespace stdsharp
{
    template<allocator_req Alloc, typename T = Alloc::value_type>
    struct allocation_data_fn
    {
        constexpr decltype(auto) operator()(const callocation<Alloc>& allocation) const noexcept
        {
            return pointer_cast<T>(std::ranges::begin(allocation));
        }

        constexpr decltype(auto) operator()(const allocation<Alloc>& allocation) const noexcept
        {
            return pointer_cast<T>(std::ranges::begin(allocation));
        }
    };

    template<allocator_req Alloc, typename T = Alloc::value_type>
    inline constexpr allocation_data_fn<Alloc, T> allocation_data{};

    template<allocator_req Alloc, typename T = Alloc::value_type>
    using allocation_cdata_fn = allocation_data_fn<Alloc, const T>;

    template<allocator_req Alloc, typename T = Alloc::value_type>
    inline constexpr allocation_cdata_fn<Alloc, T> allocation_cdata{};

    template<allocator_req Alloc, typename T = Alloc::value_type>
    struct allocation_get_fn
    {
    private:
        using data_t = allocation_data_fn<Alloc, T>;

        static constexpr data_t data{};

    public:
        constexpr decltype(auto) operator()(const auto& rng) const noexcept(noexcept(*data(rng)))
            requires std::invocable<data_t, decltype(rng)>
        {
            return *std::launder(data(rng));
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
        std::convertible_to<range_const_reference_t<Rng>, const callocation<Alloc>&>;

    template<typename Rng, typename Alloc>
    concept allocations = requires(const allocation<Alloc>& alloction) {
        requires callocations<Rng, Alloc>;
        requires std::ranges::output_range<Rng, decltype(alloction)>;
        requires std::convertible_to<range_const_reference_t<Rng>, decltype(alloction)>;
    };
}

#include "../compilation_config_out.h"

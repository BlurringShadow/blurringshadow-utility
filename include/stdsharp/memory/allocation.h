#pragma once

#include "../functional/compose.h"
#include "../ranges/ranges.h"
#include "allocator_traits.h"

namespace stdsharp::details
{
    template<typename Alloc>
    struct allocation_result_traits
    {
        using pointer = allocator_pointer<Alloc>;
        using const_pointer = allocator_const_pointer<Alloc>;
        using size_type = allocator_size_type<Alloc>;

        using callocation_result_base =
            std::ranges::subrange<const_pointer, const_pointer, std::ranges::subrange_kind::sized>;

        using allocation_result_base =
            std::ranges::subrange<pointer, const_pointer, std::ranges::subrange_kind::sized>;

        struct basic_allocation_result
        {
            using allocator_type = Alloc;
        };

        // NOLINTBEGIN(*-equals-default)
        struct callocation_result : callocation_result_base, basic_allocation_result
        {
            using callocation_result_base::callocation_result_base;

            callocation_result() = default;

            constexpr callocation_result(const const_pointer p, const size_type diff) noexcept:
                callocation_result_base(p, p + diff)
            {
            }
        };

        struct allocation_result : allocation_result_base, basic_allocation_result
        {
            using allocation_result_base::allocation_result_base;

            allocation_result() = default;

            constexpr allocation_result(const pointer p, const size_type diff) noexcept:
                allocation_result_base(p, p + diff)
            {
            }

            constexpr auto& operator=(const auto& t) noexcept
                requires std::constructible_from<
                    allocation_result_base,
                    std::ranges::iterator_t<decltype(t)>,
                    std::ranges::sentinel_t<decltype(t)>>
            {
                static_cast<allocation_result_base&>(*this) =
                    allocation_result_base{std::ranges::begin(t), std::ranges::end(t)};
                return *this;
            }

            constexpr operator callocation_result() const noexcept
            {
                return callocation_result{
                    allocation_result_base::begin(),
                    allocation_result_base::end()
                };
            }
        }; // NOLINTEND(*-equals-default)
    };
}

namespace stdsharp
{
    template<allocator_req Alloc>
    using callocation_result = details::allocation_result_traits<Alloc>::callocation_result;

    template<allocator_req Alloc>
    using allocation_result = details::allocation_result_traits<Alloc>::allocation_result;

    template<allocator_req Alloc>
    inline constexpr allocation_result<Alloc> empty_allocation_result{};
}

namespace stdsharp::details
{
    template<typename Alloc>
    struct allocation_concept
    {
    private:
        struct shadow_type
        {
            [[nodiscard]] constexpr auto begin() const noexcept
            {
                return allocator_pointer<Alloc>{nullptr};
            }

            [[nodiscard]] constexpr auto end() const noexcept
            {
                return allocator_pointer<Alloc>{nullptr};
            }

            [[nodiscard]] constexpr auto size() const noexcept
            {
                return allocator_size_type<Alloc>{0};
            }

            constexpr auto& operator=(const auto& /*unused*/) noexcept { return *this; }
        };

    public:
        template<typename T, typename LRef = std::add_lvalue_reference_t<std::decay_t<T>>>
        static constexpr auto assignable_from_shadow = nothrow_assignable_from<LRef, shadow_type>;
    };

    template<typename T, typename Alloc>
    concept allocation_common = requires(const T& t, std::remove_cvref_t<T> decay_t) {
        requires nothrow_copyable<decltype(decay_t)>;
        requires std::same_as<typename decltype(decay_t)::allocator_type, Alloc>;
        requires std::ranges::sized_range<T>;
        { std::ranges::size(t) } -> std::same_as<allocator_size_type<Alloc>>;
    };
}

namespace stdsharp
{
    template<typename T, typename Alloc>
    concept callocation = requires(const T& t) {
        requires details::allocation_common<T, Alloc>;
        { std::ranges::begin(t) } -> std::same_as<allocator_const_pointer<Alloc>>;
    };

    template<typename T, typename Alloc>
    concept allocation = requires(const T& t) {
        requires details::allocation_common<T, Alloc>;
        { std::ranges::begin(t) } -> std::same_as<allocator_pointer<Alloc>>;

        requires details::allocation_concept<Alloc>::template assignable_from_shadow<T>;

        requires nothrow_assignable_from<T&, allocation_result<Alloc>>;
    };

    template<allocator_req Alloc, typename T = Alloc::value_type>
    struct allocation_data_fn
    {
        template<typename U>
            requires(callocation<U, Alloc> || allocation<U, Alloc>)
        constexpr decltype(auto) operator()(const U& rng) const noexcept
        {
            return pointer_cast<T>(std::ranges::begin(rng));
        }
    };

    template<allocator_req Alloc, typename T = Alloc::value_type>
    inline constexpr allocation_data_fn<Alloc, T> allocation_data{};

    template<allocator_req Alloc, typename T = Alloc::value_type>
    using allocation_cdata_fn =
        composed<allocation_data_fn<Alloc, T>, cast_to_fn<allocator_const_pointer<Alloc>>>;

    template<allocator_req Alloc, typename T = Alloc::value_type>
    inline constexpr allocation_cdata_fn<Alloc, T> allocation_cdata{};

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
        callocation<range_const_reference_t<Rng>, Alloc>;

    template<typename Rng, typename Alloc>
    concept allocations = range_copyable<Rng, Rng> &&
        allocation<std::ranges::range_value_t<Rng>, Alloc>;
}

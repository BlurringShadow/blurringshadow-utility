#pragma once

#include "allocator_traits.h"
#include "stdsharp/ranges/ranges.h"

namespace stdsharp
{
    template<allocator_req Alloc>
    struct allocation_traits;

    template<typename Rng, typename Alloc>
    concept callocations = std::ranges::input_range<Rng> &&
        std::convertible_to<range_const_reference_t<Rng>,
                            const typename allocation_traits<Alloc>::callocation&>;

    template<typename Rng, typename Alloc>
    concept allocations = requires(const allocation_traits<Alloc>::allocation& alloction) {
        requires callocations<Rng, Alloc>;
        requires std::ranges::output_range<Rng, decltype(alloction)>;
        requires std::convertible_to<range_const_reference_t<Rng>, decltype(alloction)>;
    };

    template<typename Alloc, typename Src, typename Dst, typename Fn>
    concept allocation_constructible =
        std::invocable<typename allocation_traits<Alloc>::on_construct, Alloc&, Src, Dst, Fn>;

    template<typename Alloc, typename Src, typename Dst, typename Fn>
    concept allocation_assignable =
        std::invocable<typename allocation_traits<Alloc>::on_assign, Alloc&, Dst, Fn>;

    template<typename Alloc, typename Dst, typename Fn>
    concept allocation_destructible =
        std::invocable<typename allocation_traits<Alloc>::on_destroy, Alloc&, Dst, Fn>;

    template<typename Alloc, typename Src, typename Dst, typename Fn>
    concept allocation_nothrow_constructible =
        nothrow_invocable<typename allocation_traits<Alloc>::on_construct_fn, Alloc&, Src, Dst, Fn>;

    template<typename Alloc, typename Src, typename Dst, typename Fn>
    concept allocation_nothrow_assignable =
        nothrow_invocable<typename allocation_traits<Alloc>::on_assign_fn, Alloc&, Dst, Fn>;

    template<typename Alloc, typename Dst, typename Fn>
    concept allocation_nothrow_destructible =
        nothrow_invocable<typename allocation_traits<Alloc>::on_destroy_fn, Alloc&, Dst, Fn>;

    template<allocator_req Alloc>
    using callocation = allocation_traits<Alloc>::callocation;

    template<allocator_req Alloc>
    using allocation = allocation_traits<Alloc>::allocation;

    template<allocator_req Alloc>
    inline constexpr auto empty_allocation = allocation_traits<Alloc>::empty;

    template<allocator_req Alloc>
    struct allocation_traits
    {
        using allocator_type = Alloc;
        using allocator_traits = allocator_traits<allocator_type>;
        using value_type = allocator_traits::value_type;
        using size_type = allocator_traits::size_type;
        using const_pointer = allocator_traits::const_pointer;
        using cvp = allocator_traits::const_void_pointer;

    private:
        template<
            typename Ptr = const_pointer,
            typename Base =
                std::ranges::subrange<Ptr, const_pointer, std::ranges::subrange_kind::sized>>
        struct basic_allocation : Base
        {
            using Base::Base;

            using allocator_type = Alloc;
            using traits = allocation_traits;

            basic_allocation() = default;

            constexpr basic_allocation(const Ptr& p, const size_type& diff) noexcept:
                Base(p, p + diff)
            {
            }

            constexpr auto cbegin() const noexcept { return std::ranges::cbegin(*this); }

            constexpr auto cend() const noexcept { return std::ranges::cend(*this); }
        };

    public:
        struct callocation : basic_allocation<>
        {
        private:
            using m_base = basic_allocation<>;

        public:
            using m_base::m_base;
        };

        static constexpr struct allocation : basic_allocation<allocator_pointer<Alloc>>
        {
        private:
            using m_base = basic_allocation<allocator_pointer<Alloc>>;

        public:
            using m_base::m_base;

            constexpr operator callocation() const noexcept
            {
                return callocation{this->begin(), this->end()};
            }
        } empty;

        static constexpr allocation
            allocate(allocator_type& alloc, const size_type size, const cvp hint = nullptr)
        {
            return {allocator_traits::allocate(alloc, size, hint), size};
        }

        static constexpr allocation try_allocate(
            allocator_type& alloc,
            const size_type size,
            const cvp hint = nullptr
        ) noexcept
        {
            return {allocator_traits::try_allocate(alloc, size, hint), size};
        }

        static constexpr void deallocate(allocator_type& alloc, allocation& dst_allocation) noexcept
        {
            allocator_traits::deallocate(alloc, data<>(dst_allocation), dst_allocation.size());
            dst_allocation = empty;
        }
    };
}

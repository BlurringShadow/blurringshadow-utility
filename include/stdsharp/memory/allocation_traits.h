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
        using difference_type = allocator_traits::difference_type;
        using pointer = allocator_traits::pointer;
        using const_pointer = allocator_traits::const_pointer;
        using void_pointer = allocator_traits::void_pointer;
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

        template<typename T = value_type>
        struct allocation_data_fn
        {
            constexpr decltype(auto) operator()(const callocation& allocation) const noexcept
            {
                return pointer_cast<T>(allocation.begin());
            }

            constexpr decltype(auto) operator()(const allocation& allocation) const noexcept
            {
                return pointer_cast<T>(allocation.begin());
            }
        };

        template<typename T = value_type>
        static constexpr allocation_data_fn<T> data{};

        template<typename T = value_type>
        using allocation_cdata_fn = allocation_data_fn<const T>;

        template<typename T = value_type>
        static constexpr allocation_cdata_fn<T> cdata{};

        template<typename T = value_type>
        struct allocation_get_fn
        {
        private:
            using data_t = allocation_data_fn<T>;

        public:
            template<typename Allocation>
            constexpr decltype(auto) operator()(const Allocation& allocation_v) const
                noexcept(nothrow_invocable<data_t, const Allocation&>)
                requires std::invocable<data_t, const Allocation&>
            {
                return *std::launder(data_t{}(allocation_v));
            }
        };

        template<typename T = value_type>
        static constexpr allocation_get_fn<T> get{};

        template<typename T = value_type>
        using allocation_cget_fn = allocation_get_fn<const T>;

        template<typename T = value_type>
        static constexpr allocation_cget_fn<T> cget{};

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

        static constexpr void deallocate(allocator_type& alloc, allocations<Alloc> auto&& dst)
        {
            for(auto& dst_allocation : cpp_forward(dst)) deallocate(alloc, dst_allocation);
        }

        template<typename T = value_type>
        struct constructor
        {
            template<typename... Args, typename Ctor = allocator_traits::constructor>
                requires std::invocable<Ctor, allocator_type&, T*, Args...>
            constexpr void operator()(
                allocator_type& alloc,
                const allocation& allocation,
                Args&&... args
            ) const noexcept(nothrow_invocable<Ctor, allocator_type&, T*, Args...>)
            {
                Expects(allocation.size() * sizeof(value_type) >= sizeof(T));
                allocator_traits::construct(alloc, data<T>(allocation), cpp_forward(args)...);
            }
        };

        template<typename T = value_type>
        static constexpr constructor<T> construct{};

        static constexpr struct on_construct_fn
        {
            template<typename Src, allocations<Alloc> Dst, std::copy_constructible Fn>
                requires requires {
                    requires std::invocable<
                        Fn&,
                        allocator_type&,
                        range_const_reference_t<Src>,
                        range_const_reference_t<Dst>>;
                    requires(callocations<Src, Alloc> || allocations<Src, Alloc>);
                }
            constexpr void operator()(allocator_type& alloc, Src&& src, Dst&& dst, Fn fn) const
                noexcept(
                    nothrow_copy_constructible<Fn> &&
                    nothrow_invocable<
                        Fn&,
                        allocator_type&,
                        range_const_reference_t<Src>,
                        range_const_reference_t<Dst>> //
                )
            {
                for(const auto& [src_allocation, dst_allocation] :
                    std::views::zip(cpp_forward(src), cpp_forward(dst)))
                    invoke(fn, alloc, src_allocation, dst_allocation);
            }
        } on_construct{};

        static constexpr struct on_assign_fn
        {
            template<typename Src, allocations<Alloc> Dst, std::copy_constructible Fn>
                requires requires {
                    requires std::
                        invocable<Fn&, range_const_reference_t<Src>, range_const_reference_t<Dst>>;
                    requires(callocations<Src, Alloc> || allocations<Src, Alloc>);
                }
            constexpr void operator()(Src&& src, Dst&& dst, Fn fn) const noexcept(
                nothrow_copy_constructible<Fn> &&
                nothrow_invocable<
                    Fn&,
                    range_const_reference_t<Src>,
                    range_const_reference_t<Dst>> //
            )
            {
                for(const auto& [src_allocation, dst_allocation] :
                    std::views::zip(cpp_forward(src), cpp_forward(dst)))
                    invoke(fn, src_allocation, dst_allocation);
            }
        } on_assign{};

        static constexpr struct on_destroy_fn
        {
            template<allocations<Alloc> Dst, std::copy_constructible Fn>
                requires std::invocable<Fn&, allocator_type&, range_const_reference_t<Dst>>
            constexpr void operator()(allocator_type& alloc, Dst&& dst, Fn fn) const noexcept(
                nothrow_copy_constructible<Fn> &&
                nothrow_invocable<Fn&, allocator_type&, range_const_reference_t<Dst>> //
            )
            {
                for(const auto& dst_allocation : cpp_forward(dst))
                    invoke(fn, alloc, dst_allocation);
            }
        } on_destroy{};

        static constexpr void
            on_swap(allocations<Alloc> auto&& lhs, allocations<Alloc> auto&& rhs) noexcept
        {
            std::ranges::swap_ranges(lhs, rhs);
        }
    };
}

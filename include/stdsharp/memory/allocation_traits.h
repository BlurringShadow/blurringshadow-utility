#pragma once

#include "allocation.h"

namespace stdsharp
{
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
        using allocation = allocation<allocator_type>;
        using callocation = callocation<allocator_type>;

        static constexpr auto empty_result = empty_allocation<Alloc>;

        static constexpr auto size = std::ranges::size;

        static constexpr auto empty = std::ranges::empty;

        template<typename T = value_type>
        static constexpr auto data = allocation_data<Alloc, T>;

        template<typename T = value_type>
        static constexpr auto cdata = allocation_cdata<Alloc, T>;

        template<typename T = value_type>
        static constexpr auto get = allocation_get<Alloc, T>;

        template<typename T = value_type>
        static constexpr auto cget = allocation_cget<Alloc, T>;

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

        static constexpr void deallocate(allocator_type& alloc, allocations<Alloc> auto&& dst) noexcept
        {
            for(auto& dst_allocation : cpp_forward(dst))
            {
                allocator_traits::deallocate(alloc, data<>(dst_allocation), size(dst_allocation));
                dst_allocation = empty_result;
            }
        }

        template<typename T = Alloc::value_type>
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
                Expects(size(allocation) * sizeof(value_type) >= sizeof(T));
                allocator_traits::construct(alloc, data<T>(allocation), cpp_forward(args)...);
            }
        };

        template<typename T = Alloc::value_type>
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

    template<typename Alloc, typename Src, typename Dst, typename Fn>
    concept allocation_constructible = requires {
        { allocation_traits<Alloc>::on_construct } -> std::invocable<Alloc&, Src, Dst, Fn>;
    };

    template<typename Alloc, typename Src, typename Dst, typename Fn>
    concept allocation_assignable = requires {
        { allocation_traits<Alloc>::on_assign } -> std::invocable<Src, Dst, Fn>;
    };

    template<typename Alloc, typename Dst, typename Fn>
    concept allocation_destructible = requires {
        { allocation_traits<Alloc>::on_destroy } -> std::invocable<Alloc&, Dst, Fn>;
    };

    template<typename Alloc, typename Src, typename Dst, typename Fn>
    concept allocation_nothrow_constructible = requires {
        { allocation_traits<Alloc>::on_construct } -> nothrow_invocable<Alloc&, Src, Dst, Fn>;
    };

    template<typename Alloc, typename Src, typename Dst, typename Fn>
    concept allocation_nothrow_assignable = requires {
        { allocation_traits<Alloc>::on_assign } -> nothrow_invocable<Src, Dst, Fn>;
    };

    template<typename Alloc, typename Dst, typename Fn>
    concept allocation_nothrow_destructible = requires {
        { allocation_traits<Alloc>::on_destroy } -> nothrow_invocable<Alloc&, Dst, Fn>;
    };
}

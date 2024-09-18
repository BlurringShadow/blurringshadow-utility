#pragma once

#include "allocation_traits.h"

namespace stdsharp
{
    template<allocator_req Alloc>
    struct allocator_adaptor : Alloc
    {
        using allocator_type = Alloc;

        using traits = allocator_traits<allocator_type>;

        using allocation_traits = allocation_traits<Alloc>;

        using allocation = allocation_traits::allocation;

        using callocation = allocation_traits::callocation;

        constexpr allocator_type& get_allocator() noexcept { return *this; }

        constexpr const allocator_type& get_allocator() const noexcept { return *this; }

        allocator_adaptor() = default;

        constexpr allocator_adaptor(const allocator_type& other) noexcept:
            allocator_type(traits::select_on_container_copy_construction(other))
        {
        }

        constexpr allocator_adaptor(allocator_type&& other) noexcept:
            allocator_type(cpp_move(other))
        {
        }

        constexpr allocator_adaptor(const std::in_place_t /*unused*/, auto&&... args)
            noexcept(noexcept(allocator_type(cpp_forward(args)...)))
            requires requires { allocator_type(cpp_forward(args)...); }
            : allocator_type(cpp_forward(args)...)
        {
        }

    private:
        static constexpr auto always_equal_v = traits::always_equal_v;

    public:
        constexpr bool operator==(const allocator_type& other) const noexcept
        {
            return always_equal_v || get_allocator() == other;
        }

        constexpr bool operator==(const allocator_adaptor& other) const noexcept
        {
            return *this == other.get_allocator();
        }

        template<
            typename Fn,
            std::invocable<Fn&, const allocator_adaptor&> OnPropagate =
                on_propagate<traits::propagate_on_copy_v, copy_propagation, no_copy_propagation>>
        constexpr Fn& operator=(this Fn& fn, const allocator_adaptor& other)
            noexcept(nothrow_invocable<OnPropagate, Fn&, const allocator_adaptor&>)
        {
            OnPropagate{}(fn, other);
            return fn;
        }

        template<
            typename Fn,
            std::invocable<Fn&, allocator_adaptor> OnPropagate =
                on_propagate<traits::propagate_on_move_v, move_propagation, no_move_propagation>>
        constexpr Fn& operator=(this Fn& fn, allocator_adaptor&& other)
            noexcept(nothrow_invocable<OnPropagate, Fn&, allocator_adaptor>)
        {
            OnPropagate{}(fn, other);
            return fn;
        }

        template<
            typename Fn,
            typename BeforeSwap = swap_propagation<true>,
            typename AfterSwap = swap_propagation<false>>
            requires requires {
                requires traits::propagate_on_swap_v;
                requires std::invocable<Fn&, BeforeSwap>;
                requires std::invocable<Fn&, AfterSwap>;
            }
        constexpr void swap(this Fn& fn, allocator_adaptor& other) noexcept
        {
            fn(BeforeSwap{});
            std::ranges::
                swap(((allocator_adaptor&)fn).get_allocator(), other.get_allocator()); // NOLINT
            fn(AfterSwap{});
        }

        template<typename Fn>
            requires requires {
                requires !traits::propagate_on_swap_v;
                requires std::invocable<Fn&, no_swap_propagation>;
            }
        constexpr void swap(this Fn& fn, allocator_adaptor& other) noexcept
        {
            assert_equal(((allocator_adaptor&)fn).get_allocator(), other.get_allocator()); // NOLINT
            fn(no_swap_propagation{});
        }

        constexpr auto allocate(const auto size) noexcept
        {
            return allocation_traits::allocate(get_allocator(), size);
        }

        constexpr auto deallocate(const allocation& allocation) noexcept
        {
            return allocation_traits::deallocate(get_allocator(), allocation);
        }

        constexpr auto deallocate(auto&& allocations)
            requires requires {
                allocation_traits::deallocate(get_allocator(), cpp_forward(allocations));
            }
        {
            return allocation_traits::deallocate(get_allocator(), cpp_forward(allocations));
        }
    };
}

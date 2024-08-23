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

        template<bool IsEqual, bool IsBefore>
        using move_propagation = allocator_move_propagation_t<IsEqual, IsBefore>;

        template<bool IsEqual, bool IsBefore>
        using copy_propagation = allocator_copy_propagation_t<IsEqual, IsBefore>;

        template<bool IsBefore>
        using swap_propagation = allocator_swap_propagation_t<IsBefore>;

        using no_move_propagation = allocator_no_move_propagation_t;

        using no_copy_propagation = allocator_no_copy_propagation_t;

        using no_swap_propagation = allocator_no_swap_propagation_t;

        template<typename T>
        constexpr allocator_type& get_allocator(this T&& t) noexcept
        {
            return fwd_cast<T, allocator_adaptor>(t);
        }

        allocator_adaptor() = default;

        constexpr allocator_adaptor(const allocator_adaptor& other) noexcept:
            allocator_adaptor(other.get_allocator())
        {
        }

        constexpr allocator_adaptor(allocator_adaptor&& other) noexcept:
            allocator_adaptor(cpp_move(other).get_allocator())
        {
        }

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

        allocator_adaptor& operator=(const allocator_adaptor&) = delete;
        allocator_adaptor& operator=(allocator_adaptor&&) = delete;

        ~allocator_adaptor() = default;

    private:
        static constexpr auto always_equal_v = traits::always_equal_v;

    public:
        constexpr bool is_equal(const allocator_type& other) const noexcept
        {
            return always_equal_v || get_allocator() == other;
        }

    private:
        template<bool IsEqual, template<bool, bool> typename Propagation>
        struct on_assign
        {
            using before_propagation = Propagation<IsEqual, true>;
            using after_propagation = Propagation<IsEqual, false>;

            template<std::invocable<before_propagation> Fn>
                requires std::invocable<Fn, after_propagation>
            constexpr void operator()(Fn& fn, auto&& other) const //
                noexcept(nothrow_invocable<Fn, before_propagation> && nothrow_invocable<Fn, after_propagation>)
            {
                fn(Propagation<IsEqual, true>{});
                ((allocator_adaptor&)fn).get_allocator() = // NOLINT
                    cpp_forward(other).get_allocator();
                fn(Propagation<IsEqual, false>{});
            }
        };

        template<bool Propagate, template<bool, bool> typename Propagation, typename NoPropagation>
        struct on_propagate
        {
            using assign = on_assign<true, Propagation>;
            using unequal_assign = on_assign<false, Propagation>;

            template<typename Fn, typename Other>
                requires requires {
                    requires Propagate;
                    requires std::invocable<assign, Fn&, Other>;
                }
            constexpr void operator()(Fn& fn, Other&& other) const
                noexcept(nothrow_invocable<assign, Fn&, Other>)
            {
                assign{}(fn, cpp_forward(other));
            }

            template<typename Fn, typename Other>
                requires requires {
                    requires Propagate;
                    requires !always_equal_v;
                    requires std::invocable<assign, Fn&, Other>;
                    requires std::invocable<unequal_assign, Fn&, Other>;
                }
            constexpr void operator()(Fn& fn, Other&& other) const
                noexcept(nothrow_invocable<assign, Fn&, Other> && nothrow_invocable<unequal_assign, Fn&, Other>)
            {
                if(((allocator_adaptor&)fn).get_allocator() != other.get_allocator()) // NOLINT
                {
                    unequal_assign{}(fn, cpp_forward(other));
                    return;
                }

                assign{}(fn, cpp_forward(other));
            }

            template<typename Fn>
                requires requires {
                    requires !Propagate;
                    requires std::invocable<Fn&, NoPropagation>;
                }
            constexpr void operator()(Fn& fn, auto&& /*unused*/)
                noexcept(nothrow_invocable<Fn&, NoPropagation>)
            {
                fn(NoPropagation{});
            }
        };

    public:
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

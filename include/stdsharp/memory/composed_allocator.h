#pragma once

#include "../exception/exception.h"
#include "allocator_traits.h"

namespace stdsharp
{
    template<typename T>
    concept allocator_contains = requires(const T& alloc, const allocator_pointer<T> p) {
        { alloc.contains(p) } noexcept -> nothrow_boolean_testable;
    };

    template<allocator_contains FirstAlloc, allocator_req SecondAlloc>
        requires requires(FirstAlloc::value_type value, const void* ptr) {
            requires std::same_as<decltype(value), typename SecondAlloc::value_type>;
            {
                pointer_traits<typename allocator_traits<FirstAlloc>::const_void_pointer>::
                    to_pointer(ptr)
            } noexcept;

            {
                pointer_traits<typename allocator_traits<SecondAlloc>::const_void_pointer>::
                    to_pointer(ptr)
            } noexcept;
        }
    class composed_allocator : indexed_values<FirstAlloc, SecondAlloc>
    {
    public:
        using first_allocator_type = FirstAlloc;
        using second_allocator_type = SecondAlloc;
        using value_type = FirstAlloc::value_type;

    private:
        using indexed_values = indexed_values<FirstAlloc, SecondAlloc>;

        using first_traits = allocator_traits<first_allocator_type>;
        using second_traits = allocator_traits<second_allocator_type>;

        using first_ptr = first_traits::pointer;
        using second_ptr = second_traits::pointer;

        using first_cvp = first_traits::const_void_pointer;
        using second_cvp = second_traits::const_void_pointer;

        using first_ptr_traits = pointer_traits<first_ptr>;
        using second_ptr_traits = pointer_traits<second_ptr>;

        using first_cvp_traits = pointer_traits<first_cvp>;
        using second_cvp_traits = pointer_traits<second_cvp>;

        static constexpr auto pointer_cast = stdsharp::pointer_cast<value_type>;

    public:
        using propagate_on_container_copy_assignment = std::disjunction<
            typename first_traits::propagate_on_container_copy_assignment,
            typename second_traits::propagate_on_container_copy_assignment>;
        using propagate_on_container_move_assignment = std::disjunction<
            typename first_traits::propagate_on_container_move_assignment,
            typename second_traits::propagate_on_container_move_assignment>;
        using propagate_on_container_swap = std::disjunction<
            typename first_traits::propagate_on_container_swap,
            typename second_traits::propagate_on_container_swap>;

        using is_always_equal = std::conjunction<
            typename first_traits::is_always_equal,
            typename second_traits::is_always_equal>;

        template<typename T>
        struct rebind
        {
            using other = composed_allocator<
                typename first_traits::template rebind_alloc<T>,
                typename second_traits::template rebind_alloc<T>>;
        };

        composed_allocator() = default;

        template<typename... Args>
            requires std::constructible_from<indexed_values, Args...>
        constexpr explicit(sizeof...(Args) == 1) composed_allocator(Args&&... args)
            noexcept(nothrow_constructible_from<indexed_values, Args...>):
            indexed_values(cpp_forward(args)...)
        {
        }

        [[nodiscard]] constexpr auto allocate(const std::size_t n, const void* const hint = nullptr)
        {
            const auto ptr = first_traits::
                try_allocate(get_first_allocator(), n, first_cvp_traits::to_pointer(hint));
            return ptr == nullptr ? //
                second_traits::
                    allocate(get_second_allocator(), n, second_cvp_traits::to_pointer(hint)) :
                ptr;
        }

        [[nodiscard]] constexpr auto
            try_allocate(const std::size_t n, const void* const hint = nullptr) noexcept
        {
            const auto ptr = first_traits::
                try_allocate(get_first_allocator(), n, first_cvp_traits::to_pointer(hint));
            return ptr == nullptr ? //
                second_traits::
                    try_allocate(get_second_allocator(), n, second_cvp_traits::to_pointer(hint)) :
                ptr;
        }

        constexpr void deallocate(value_type* const ptr, const std::size_t n) noexcept
        {
            auto& first = get_first_allocator();
            if(auto first_ptr_v = pointer_traits<first_ptr>::to_pointer(ptr);
               first.contains(first_ptr_v))
                first.deallocate(ptr, n);
            else get_second_allocator().deallocate(pointer_traits<second_ptr>::to_pointer(ptr), n);
        }

        [[nodiscard]] constexpr auto max_size() const noexcept
        {
            return std::min( //
                first_traits::max_size(get_first_allocator()),
                second_traits::max_size(get_second_allocator())
            );
        }

        [[nodiscard]] constexpr composed_allocator select_on_container_copy_construction() const
        {
            return {
                first_traits::select_on_container_copy_construction(get_first_allocator()),
                second_traits::select_on_container_copy_construction(get_second_allocator())
            };
        }

        [[nodiscard]] constexpr auto allocate_at_least(const std::size_t n)
        {
            struct
            {
                value_type* ptr;
                std::size_t count;
            } result;

            try
            {
                const auto& res = first_traits::allocate_at_least(get_first_allocator(), n);
                result = {.ptr = first_ptr_traits::to_address(res.ptr), .count = res.count};
            }
            catch(...)
            {
                auto&& first_ptr = std::current_exception();
                try
                {
                    const auto& res = second_traits::allocate_at_least(get_second_allocator(), n);
                    result = {.ptr = second_ptr_traits::to_address(res.ptr), .count = res.count};
                }
                catch(...)
                {
                    throw aggregate_exceptions{first_ptr, std::current_exception()};
                }
            }

            return result;
        }

        template<
            typename T,
            typename... Args,
            typename FirstCtor = first_traits::constructor,
            typename SecondCtor = second_traits::constructor>
            requires std::invocable<FirstCtor, first_allocator_type&, T*, Args...> &&
            std::invocable<SecondCtor, second_allocator_type&, T*, Args...>
        constexpr decltype(auto) construct(T* const ptr, Args&&... args) noexcept(
            nothrow_invocable<FirstCtor, first_allocator_type&, T*, Args...> &&
            nothrow_invocable<SecondCtor, second_allocator_type&, T*, Args...> //
        )
        {
            if(auto& first = get_first_allocator();
               first.contains(pointer_traits<first_ptr>::to_pointer(pointer_cast(ptr))))
                return first_traits::construct(first, ptr, cpp_forward(args)...);
            return second_traits::construct(get_second_allocator(), ptr, cpp_forward(args)...);
        }

        [[nodiscard]] constexpr bool contains(const value_type* const ptr) const noexcept
            requires allocator_contains<SecondAlloc>
        {
            const auto vp = static_cast<const void*>(ptr);
            return get_first_allocator().contains(pointer_traits<first_ptr>::to_pointer(vp)) ||
                get_second_allocator().contains(pointer_traits<second_ptr>::to_pointer(vp));
        }

        [[nodiscard]] bool operator==(const composed_allocator&) const noexcept = default;

        [[nodiscard]] constexpr auto& get_first_allocator() const noexcept
        {
            return indexed_values::template get<0>();
        }

        [[nodiscard]] constexpr auto& get_first_allocator() noexcept
        {
            return indexed_values::template get<0>();
        }

        [[nodiscard]] constexpr auto& get_second_allocator() const noexcept
        {
            return indexed_values::template get<1>();
        }

        [[nodiscard]] constexpr auto& get_second_allocator() noexcept
        {
            return indexed_values::template get<1>();
        }
    };

    template<typename T, typename U>
    composed_allocator(T&&, U&&) -> composed_allocator<std::decay_t<T>, std::decay_t<U>>;
}

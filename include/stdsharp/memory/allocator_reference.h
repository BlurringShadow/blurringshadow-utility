#pragma once

#include "allocator_traits.h"

namespace stdsharp
{
    template<allocator_req Alloc>
    class allocator_reference : std::reference_wrapper<Alloc>
    {
        using traits = allocator_traits<Alloc>;

    public:
        using std::reference_wrapper<Alloc>::reference_wrapper;

        using value_type = traits::value_type;
        using pointer = traits::pointer;
        using const_pointer = traits::const_pointer;
        using void_pointer = traits::void_pointer;
        using const_void_pointer = traits::const_void_pointer;
        using difference_type = traits::difference_type;
        using size_type = traits::size_type;
        using propagate_on_container_copy_assignment =
            traits::propagate_on_container_copy_assignment;
        using propagate_on_container_move_assignment =
            traits::propagate_on_container_move_assignment;
        using propagate_on_container_swap = traits::propagate_on_container_swap;
        using is_always_equal = traits::is_always_equal;

        template<typename U>
        struct rebind
        {
            using other = allocator_reference<typename traits::template rebind_alloc<U>>;
        };

        [[nodiscard]] constexpr auto
            allocate(const size_type n, const const_void_pointer hint = nullptr) const
        {
            return traits::allocate(this->get(), n, hint);
        }

        [[nodiscard]] constexpr auto
            try_allocate(const size_type n, const const_void_pointer hint = nullptr) const
        {
            return traits::try_allocate(this->get(), n, hint);
        }

        [[nodiscard]] constexpr auto allocate_at_least(const size_type n) const
            requires requires { traits::allocate_at_least(this->get(), n); }
        {
            return traits::allocate_at_least(this->get(), n);
        }

        template<typename U, typename... Args>
        constexpr decltype(auto) construct(U* const p, Args&&... args) const
            noexcept(noexcept(traits::construct(this->get(), p, std::declval<Args>()...)))
        {
            return traits::construct(this->get(), p, cpp_forward(args)...);
        }

        template<typename U>
        constexpr void destroy(U* const p) const noexcept(noexcept(traits::destroy(this->get(), p)))
            requires requires { traits::destroy(this->get(), p); }
        {
            traits::destroy(this->get(), p);
        }

        constexpr void deallocate(const pointer p, const size_type n) const noexcept
        {
            traits::deallocate(this->get(), p, n);
        }

        [[nodiscard]] constexpr auto max_size() const noexcept
        {
            return traits::max_size(this->get());
        }

        [[nodiscard]] constexpr auto select_on_container_copy_construction() const
            noexcept(noexcept(traits::select_on_container_copy_construction(this->get())))
        {
            return traits::select_on_container_copy_construction(this->get());
        }

        [[nodiscard]] constexpr bool operator==(const allocator_reference& other) const noexcept
        {
            return this->get() == other.get();
        }
    };

    template<typename Alloc>
    allocator_reference(Alloc&) -> allocator_reference<Alloc>;
}

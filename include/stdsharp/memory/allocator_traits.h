#pragma once

#include "../cassert/cassert.h"
#include "../type_traits/object.h"
#include "pointer_traits.h"

#include <utility>

namespace stdsharp::details
{
    struct allocator_concept_traits
    {
    private:
        struct shadow_type
        {
            std::uintmax_t v;
            std::uintmax_t w;
        };

    public:
        template<typename Traits>
        using rebind_traits = Traits::template rebind_traits<shadow_type>;
    };

    template<typename T>
    concept is_allocator_pointer = nullable_pointer<T> && //
        nothrow_default_initializable<T> && //
        nothrow_swappable<T> && //
        nothrow_copyable<T> && //
        nothrow_weakly_equality_comparable<T> &&
        nothrow_weakly_equality_comparable_with<T, std::nullptr_t>;

    template<
        typename Alloc,
        typename TTraits = std::allocator_traits<Alloc>,
        typename Ptr = TTraits::pointer,
        typename ConstPtr = TTraits::const_pointer,
        typename VoidPtr = TTraits::void_pointer,
        typename CVP = TTraits::const_void_pointer,
        typename Value = TTraits::value_type,
        typename Size = TTraits::size_type,
        typename UTraits = allocator_concept_traits::rebind_traits<TTraits>,
        typename UAlloc = UTraits::allocator_type,
        typename AlwaysEqual = TTraits::is_always_equal,
        typename CopyAssign = TTraits::propagate_on_container_copy_assignment,
        typename MoveAssign = TTraits::propagate_on_container_move_assignment,
        typename Swap = TTraits::propagate_on_container_swap>
    concept allocator_req = requires(
        Alloc alloc,
        Value v,
        Ptr p,
        ConstPtr const_p,
        CVP cvp,
        Size size,
        TTraits t_traits,
        UTraits::pointer other_p,
        UTraits::const_pointer other_const_p,
        UTraits::allocator_type u_alloc
    ) {
        nothrow_copyable<Alloc>;

        requires !const_volatile<Value>;

        requires is_allocator_pointer<Ptr> && is_allocator_pointer<ConstPtr> &&
            is_allocator_pointer<VoidPtr> && is_allocator_pointer<CVP>;

        requires std::random_access_iterator<Ptr> && std::contiguous_iterator<Ptr>;

        requires nothrow_convertible_to<Ptr, ConstPtr> && std::random_access_iterator<ConstPtr> &&
            std::contiguous_iterator<ConstPtr>;

        requires nothrow_convertible_to<Ptr, VoidPtr> &&
            nothrow_explicitly_convertible<VoidPtr, Ptr> &&
            std::same_as<VoidPtr, typename UTraits::void_pointer>;

        requires nothrow_convertible_to<Ptr, CVP> && nothrow_convertible_to<ConstPtr, CVP> &&
            nothrow_explicitly_convertible<CVP, ConstPtr> && nothrow_convertible_to<VoidPtr, CVP> &&
            std::same_as<CVP, typename UTraits::const_void_pointer>;

        requires std::unsigned_integral<Size>;

        requires std::signed_integral<typename TTraits::difference_type>;

        requires std::same_as<typename UTraits::template rebind_alloc<Value>, Alloc>;

        { *p } -> std::same_as<std::add_lvalue_reference_t<Value>>;
        { *const_p } -> std::same_as<add_const_lvalue_ref_t<Value>>;
        { *const_p } -> std::same_as<add_const_lvalue_ref_t<Value>>;

        requires requires() {
            nothrow_constructible_from<Alloc, const UAlloc&>;
            nothrow_constructible_from<Alloc, UAlloc>;

            { other_p->v } -> std::same_as<decltype(((*other_p).v))>;
            { other_const_p->v } -> std::same_as<decltype(((*other_const_p).v))>;

            t_traits.construct(alloc, other_p);
            t_traits.destroy(alloc, other_p);
        };

        { alloc == alloc, alloc != alloc } noexcept;

        stdsharp::pointer_traits<Ptr>::pointer_to(v);

        { t_traits.allocate(alloc, size) } -> std::same_as<Ptr>;
        { t_traits.allocate(alloc, size, cvp) } -> std::same_as<Ptr>;

        alloc.deallocate(p, size);

        { t_traits.max_size(alloc) } noexcept -> std::same_as<Size>;

        requires std::derived_from<AlwaysEqual, std::true_type> ||
            std::derived_from<AlwaysEqual, std::false_type>;

        { t_traits.select_on_container_copy_construction(alloc) } -> std::same_as<Alloc>;

        requires std::derived_from<CopyAssign, std::true_type> && nothrow_copy_assignable<Alloc> ||
                std::derived_from<CopyAssign, std::false_type>;

        requires std::derived_from<MoveAssign, std::true_type> && nothrow_move_assignable<Alloc> ||
                std::derived_from<MoveAssign, std::false_type>;

        requires std::derived_from<Swap, std::true_type> && nothrow_swappable<Alloc> ||
                std::derived_from<Swap, std::false_type>;
    };
}

namespace stdsharp
{
    using max_alignment = std::alignment_of<std::max_align_t>;

    inline constexpr auto max_alignment_v = max_alignment::value;

    template<typename T>
    concept over_aligned = alignof(T) > max_alignment_v;

    template<typename Alloc>
    concept allocator_req = details::allocator_req<Alloc>;

    template<auto = -1>
    struct allocator_propagation
    {
    };

    template<bool IsEqual>
    struct allocator_propagation<IsEqual>
    {
        static constexpr auto is_equal = IsEqual;

        bool assigned;
    };

    template<allocator_req Alloc>
    struct allocator_traits : private std::allocator_traits<Alloc>
    {
    private: // NOLINTBEGIN(*-owning-memory)
        struct no_nullptr_fn
        {
            constexpr void operator()(Alloc&, std::nullptr_t, auto&&...) const = delete;
        };

        struct default_constructor
        {
            template<typename T, typename... Args>
            constexpr T* operator()(const Alloc& /*unused*/, T* const ptr, Args&&... args) const
                noexcept(nothrow_constructible_from<T, Args...>)
                requires std::constructible_from<T, Args...>
            {
                assert_not_null(ptr);
                return std::ranges::construct_at(ptr, cpp_forward(args)...);
            }

            template<typename T>
            constexpr T* operator()(
                const Alloc& /*unused*/,
                void* const ptr,
                const std::in_place_type_t<T> /*unused*/,
                auto&&... args
            ) const noexcept(noexcept(::new(ptr) T{cpp_forward(args)...}))
                requires requires { ::new(ptr) T{cpp_forward(args)...}; }
            {
                assert_not_null(ptr);
                return ::new(ptr) T{cpp_forward(args)...};
            }
        };

        struct custom_constructor
        {
            template<typename T>
            constexpr T* operator()(Alloc& a, T* const ptr, auto&&... args) const
                noexcept(noexcept(a.construct(ptr, cpp_forward(args)...)))
                requires requires { a.construct(ptr, cpp_forward(args)...); }
            {
                assert_not_null(ptr);
                return a.construct(ptr, cpp_forward(args)...);
            }
        }; // NOLINTEND(*-owning-memory)

        using m_base = std::allocator_traits<Alloc>;

    public:
        using constructor =
            sequenced_invocables<no_nullptr_fn, custom_constructor, default_constructor>;

        static constexpr constructor construct{};

    private:
        struct valid_constructor_uses_allocator
        {
            template<typename... Args>
                requires std::invocable<constructor, Alloc&, Args..., const Alloc&>
            constexpr auto* operator()(Alloc& alloc, Args&&... args) const
                noexcept(nothrow_invocable<constructor, Alloc&, Args..., const Alloc&>)
            {
                return construct(alloc, cpp_forward(args)..., std::as_const(alloc));
            }

            template<typename T, typename... Args, typename Tag = std::allocator_arg_t>
                requires std::invocable<constructor, Alloc&, T*, Tag, const Alloc&, Args...>
            constexpr auto* operator()(Alloc& alloc, T* const ptr, Args&&... args) const
                noexcept(nothrow_invocable<constructor, Alloc&, T*, Tag, const Alloc&, Args...>)
            {
                return construct(
                    alloc,
                    ptr,
                    std::allocator_arg,
                    std::as_const(alloc),
                    cpp_forward(args)...
                );
            }
        };

    public:
        using constructor_uses_allocator =
            sequenced_invocables<no_nullptr_fn, valid_constructor_uses_allocator>;

        static constexpr constructor_uses_allocator construct_uses_allocator{};

    private:
        struct custom_destructor
        {
            template<typename T>
            constexpr void operator()(Alloc& a, T* const ptr) const noexcept
                requires requires { a.destroy(ptr); }
            {
                assert_not_null(ptr);
                a.destroy(ptr);
            }
        };

        struct default_destructor
        {
            template<typename T>
            constexpr void operator()(const Alloc& /*unused*/, T* const ptr) const noexcept
            {
                assert_not_null(ptr);
                std::ranges::destroy_at(ptr);
            }
        };

    public:
        using destructor =
            sequenced_invocables<no_nullptr_fn, custom_destructor, default_destructor>;

        static constexpr destructor destroy{};

        using typename m_base::allocator_type;
        using typename m_base::const_pointer;
        using typename m_base::const_void_pointer;
        using typename m_base::difference_type;
        using typename m_base::is_always_equal;
        using typename m_base::pointer;
        using typename m_base::propagate_on_container_copy_assignment;
        using typename m_base::propagate_on_container_move_assignment;
        using typename m_base::propagate_on_container_swap;
        using typename m_base::size_type;
        using typename m_base::value_type;
        using typename m_base::void_pointer;

#if __cpp_lib_allocate_at_least >= 202302L
        using m_base::allocate_at_least;
#endif

        static constexpr auto propagate_on_copy_v = propagate_on_container_copy_assignment::value;

        static constexpr auto propagate_on_move_v = propagate_on_container_move_assignment::value;

        static constexpr auto propagate_on_swap_v = propagate_on_container_swap::value;

        static constexpr auto always_equal_v = is_always_equal::value;

        template<typename U>
        using rebind_alloc = m_base::template rebind_alloc<U>;

        template<typename U>
        using rebind_traits = m_base::template rebind_traits<U>;

        using m_base::max_size;

        [[nodiscard]] static constexpr allocator_type
            select_on_container_copy_construction(const allocator_type& alloc) noexcept
        {
            return alloc;
        }

        [[nodiscard]] static constexpr allocator_type
            select_on_container_copy_construction(const allocator_type& alloc)
                noexcept(noexcept(alloc.select_on_container_copy_construction()))
            requires requires {
                {
                    alloc.select_on_container_copy_construction()
                } -> std::convertible_to<allocator_type>;
            }
        {
            return alloc.select_on_container_copy_construction();
        }

        [[nodiscard]] static constexpr pointer allocate(
            allocator_type& alloc,
            const size_type count,
            const const_void_pointer hint = nullptr
        )
        {
            return hint == nullptr ? m_base::allocate(alloc, count) :
                                     m_base::allocate(alloc, count, hint);
        }

        static constexpr void deallocate(allocator_type&, std::nullptr_t, size_type) = delete;

        static constexpr void
            deallocate(allocator_type& alloc, const pointer ptr, const size_type count) noexcept
        {
            assert_not_null(ptr);
            m_base::deallocate(alloc, ptr, count);
        }

        [[nodiscard]] static constexpr pointer try_allocate(
            allocator_type& alloc,
            const size_type count,
            const const_void_pointer hint = nullptr
        ) noexcept
        {
            if(max_size(alloc) < count) return pointer{};

            try
            {
                return allocate(alloc, count, hint);
            }
            catch(...)
            {
                return pointer{};
            }
        }

        [[nodiscard]] static constexpr auto try_allocate(
            allocator_type& alloc,
            const size_type count,
            [[maybe_unused]] const const_void_pointer hint = nullptr
        ) noexcept
            requires requires {
                { alloc.try_allocate(count) } noexcept -> std::same_as<pointer>;
            }
        {
            if constexpr(requires {
                             { alloc.try_allocate(count, hint) } noexcept -> std::same_as<pointer>;
                         })
                if(hint != nullptr) return alloc.try_allocate(count, hint);

            return alloc.try_allocate(count);
        }

        using move_propagation = std::conditional_t<
            propagate_on_move_v,
            allocator_propagation<always_equal_v>,
            allocator_propagation<>>;

        using copy_propagation = std::conditional_t<
            propagate_on_copy_v,
            allocator_propagation<always_equal_v>,
            allocator_propagation<>>;

        template<typename T>
        static constexpr lifetime_req type_req{
            get_expr_req(std::invocable<constructor, allocator_type&, T*>, nothrow_invocable<constructor, allocator_type&, T*>),
            get_expr_req(std::invocable<constructor, allocator_type&, T*, T>, nothrow_invocable<constructor, allocator_type&, T*, T>),
            get_expr_req(std::invocable<constructor, allocator_type&, T*, const T&>, nothrow_invocable<constructor, allocator_type&, T*, const T&>),
            get_expr_req(move_assignable<T>, nothrow_move_assignable<T>),
            get_expr_req(copy_assignable<T>, nothrow_copy_assignable<T>),
            expr_req::no_exception,
            get_expr_req(std::swappable<T>, nothrow_swappable<T>),
        };
    };

    template<allocator_req Alloc>
    using allocator_pointer = allocator_traits<Alloc>::pointer;

    template<allocator_req Alloc>
    using allocator_const_pointer = allocator_traits<Alloc>::const_pointer;

    template<allocator_req Alloc>
    using allocator_value_type = allocator_traits<Alloc>::value_type;

    template<allocator_req Alloc>
    using allocator_size_type = allocator_traits<Alloc>::size_type;

    template<allocator_req Alloc>
    using allocator_cvp = allocator_traits<Alloc>::const_void_pointer;

    template<allocator_req Alloc>
    using allocator_void_pointer = allocator_traits<Alloc>::void_pointer;

    template<allocator_req Alloc>
    using allocator_difference_type = allocator_traits<Alloc>::difference_type;

    template<typename>
    struct allocator_of;

    template<typename T>
        requires requires { typename T::allocator_type; }
    struct allocator_of<T> : std::type_identity<typename T::allocator_type>
    {
    };

    template<typename T>
    using allocator_of_t = allocator_of<T>::type;

    template<typename Alloc, typename Fn>
    concept allocator_move_assignable = std::copy_constructible<Fn> &&
        std::invocable<Fn&, const typename allocator_traits<Alloc>::move_propagation&, Alloc&>;

    template<typename Alloc, typename Fn>
    concept allocator_nothrow_move_assignable = nothrow_copy_constructible<Fn> &&
        nothrow_invocable<Fn&, const typename allocator_traits<Alloc>::move_propagation&, Alloc&>;

    template<typename Alloc, typename Fn>
    concept allocator_copy_assignable = std::copy_constructible<Fn> &&
        std::invocable<Fn&, const typename allocator_traits<Alloc>::copy_propagation&, Alloc&>;

    template<typename Alloc, typename Fn>
    concept allocator_nothrow_copy_assignable = nothrow_copy_constructible<Fn> &&
        nothrow_invocable<Fn&, const typename allocator_traits<Alloc>::copy_propagation&, Alloc&>;

    template<allocator_req Alloc>
    struct allocator_adaptor : Alloc
    {
        using allocator_type = Alloc;
        using traits = allocator_traits<allocator_type>;

        allocator_adaptor() = default;

        [[nodiscard]] constexpr allocator_type& get_allocator() noexcept { return *this; }

        [[nodiscard]] constexpr const allocator_type& get_allocator() const noexcept
        {
            return *this;
        }

        constexpr allocator_adaptor(const std::in_place_t /*unused*/, auto&&... args)
            noexcept(noexcept(allocator_type(cpp_forward(args)...)))
            requires requires { allocator_type(cpp_forward(args)...); }
            : allocator_type(cpp_forward(args)...)
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

    private:
        template<bool Propagate, bool IsEqual>
        using on_assign =
            std::conditional_t<Propagate, allocator_propagation<IsEqual>, allocator_propagation<>>;

        static constexpr auto always_equal_v = traits::always_equal_v;

        constexpr void equal_assign(auto&& other, auto& fn)
        {
            using on_assign = on_assign<true, true>;

            invoke(fn, on_assign{false}, get_allocator());
            get_allocator() = cpp_forward(other);
            invoke(fn, on_assign{true}, get_allocator());
        }

        constexpr void not_equal_assign(auto&& other, auto& fn)
        {
            using on_assign = on_assign<true, false>;

            invoke(fn, on_assign{false}, get_allocator());
            get_allocator() = cpp_forward(other);
            invoke(fn, on_assign{true}, get_allocator());
        }

        constexpr void no_assign(auto& fn)
        {
            invoke(fn, on_assign<false, false>{}, get_allocator());
        }

    public:
        template<typename T>
        constexpr void assign(const allocator_type& other, T fn)
            noexcept(allocator_nothrow_copy_assignable<allocator_type, T>)
            requires allocator_copy_assignable<allocator_type, T>
        {
            if constexpr(traits::propagate_on_copy_v)
                if constexpr(always_equal_v) equal_assign(other, fn);
                else
                {
                    if(get_allocator() == other) equal_assign(other, fn);
                    else not_equal_assign(other, fn);
                }
            else no_assign(fn);
        }

        template<typename T>
        constexpr void assign(allocator_type&& other, T fn)
            noexcept(allocator_nothrow_move_assignable<allocator_type, T>)
            requires allocator_move_assignable<allocator_type, T>
        {
            if constexpr(traits::propagate_on_copy_v)
                if constexpr(always_equal_v) equal_assign(cpp_move(other), fn);
                else
                {
                    if(get_allocator() == other) equal_assign(cpp_move(other), fn);
                    else not_equal_assign(cpp_move(other), fn);
                }
            else no_assign(fn);
        }

        constexpr void swap_with(allocator_type& other) noexcept
        {
            if constexpr(traits::propagate_on_swap_v)
                std::ranges::swap(get_allocator(), other.get_allocator());
            else if constexpr(!always_equal_v) Expects(get_allocator() == other.get_allocator());
        }
    };
}

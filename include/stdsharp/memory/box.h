#pragma once

#include "allocation_value.h"
#include "../cmath/cmath.h"

namespace stdsharp::details
{
    template<lifetime_req Req, allocator_req Alloc>
    struct box_traits
    {
        using allocation_traits = allocation_traits<Alloc>;

        using allocator_type = allocation_traits::allocator_type;

        using allocator_traits = allocation_traits::allocator_traits;

        using allocation_type = allocation_traits::allocation_result;

        using callocation_type = allocation_traits::callocation_result;

        using fake_type = fake_type<Req>;

        static constexpr auto req = allocator_traits::template type_req<fake_type>;

        using dispatchers = invocables<
            dispatcher<
                req.copy_construct,
                void,
                allocator_type&,
                const callocation_type&,
                const allocation_type&>,
            dispatcher<
                req.move_construct,
                void,
                allocator_type&,
                const allocation_type&,
                const allocation_type&>,
            dispatcher<req.copy_assign, void, const callocation_type&, const allocation_type&>,
            dispatcher<req.move_assign, void, const allocation_type&, const allocation_type&>,
            dispatcher<req.destruct, void, allocator_type&, const allocation_type&>>;

        using allocation_value = allocation_value<allocator_type, box_traits>;

        using allocations_type = std::array<allocation_type, 1>;
        using callocations_type =
            cast_view<std::ranges::ref_view<const allocations_type>, callocation_type>;
    };

    template<lifetime_req Req, typename Alloc, typename T>
    concept box_type_compatible = std::constructible_from<
        typename box_traits<Req, Alloc>::dispatchers,
        allocation_value<Alloc, T>,
        allocation_value<Alloc, T>,
        allocation_value<Alloc, T>,
        allocation_value<Alloc, T>,
        allocation_value<Alloc, T>>;

    template<lifetime_req Req, typename Alloc, lifetime_req OtherReq>
    concept box_compatible =
        box_type_compatible<Req, Alloc, typename box_traits<Req, Alloc>::fake_type>;
}

namespace stdsharp
{
    template<typename Alloc, lifetime_req Req>
    class allocation_value<Alloc, details::box_traits<Req, Alloc>> :
        details::box_traits<Req, Alloc>::dispatchers
    {
        using m_dispatchers = details::box_traits<Req, Alloc>::dispatchers;

        std::size_t value_size_{};

        template<std::size_t I>
        [[nodiscard]] constexpr bool equal_to(const allocation_value& other) const noexcept
        {
            return cpo::get_element<I>(static_cast<const m_dispatchers&>(*this)) ==
                cpo::get_element<I>(static_cast<const m_dispatchers&>(other));
        }

    public:
        using m_dispatchers::operator();

        allocation_value() = default;

        constexpr bool operator==(const allocation_value& other) const noexcept
        {
            return equal_to<0>(other) && equal_to<1>(other) && equal_to<2>(other) &&
                equal_to<3>(other) && equal_to<4>(other);
        }

        template<typename T, typename Op = allocation_value<Alloc, T>>
            requires details::box_type_compatible<Req, Alloc, T>
        explicit constexpr allocation_value(const std::in_place_type_t<T> /*unused*/) noexcept:
            m_dispatchers(Op{}, Op{}, Op{}, Op{}, Op{}), value_size_(sizeof(T))
        {
        }

        template<lifetime_req OtherReq>
            requires details::box_compatible<Req, Alloc, OtherReq> && (Req != OtherReq)
        explicit constexpr allocation_value(
            const allocation_value<Alloc, details::box_traits<OtherReq, Alloc>> other
        ) noexcept:
            m_dispatchers(other, other, other, other, other), value_size_(other.value_size_)
        {
        }

        [[nodiscard]] constexpr auto value_size() const noexcept { return value_size_; }
    };

    template<lifetime_req Req, allocator_req Alloc>
    class box : details::box_traits<Req, Alloc> // NOLINTBEGIN(*-noexcept-*)
    {
        using traits = details::box_traits<Req, Alloc>;

        template<lifetime_req, allocator_req>
        friend class box;

        friend constexpr void swap(box& lhs, box& rhs) noexcept
        {
            lhs.alloc_adaptor_.swap_with(rhs.get_allocator());
            std::swap(lhs.get_allocation(), rhs.get_allocation());
            std::swap(lhs.allocation_value_, rhs.allocation_value_);
        }

        using typename traits::allocation_traits;

    public:
        using typename traits::allocator_type;

    private:
        using typename traits::allocator_traits;
        using typename traits::allocation_type;
        using typename traits::callocation_type;
        using typename traits::allocation_value;
        using typename traits::allocations_type;
        using typename traits::callocations_type;

        allocator_adaptor<allocator_type> alloc_adaptor_{};
        allocations_type allocations_;
        allocation_value allocation_value_{};

        [[nodiscard]] constexpr auto& get_allocations() noexcept { return allocations_; }

        [[nodiscard]] constexpr auto& get_allocations() const noexcept { return allocations_; }

        [[nodiscard]] constexpr auto& get_allocations_view() noexcept { return get_allocations(); }

        [[nodiscard]] constexpr auto get_allocations_view() const noexcept
        {
            return callocations_type{std::ranges::ref_view{get_allocations()}, {}};
        }

        [[nodiscard]] constexpr auto& get_allocation() noexcept
        {
            return get_allocations().front();
        }

        [[nodiscard]] constexpr auto& get_allocation() const noexcept
        {
            return get_allocations().front();
        }

        [[nodiscard]] constexpr auto& get_allocator() noexcept
        {
            return alloc_adaptor_.get_allocator();
        }

    public:
        [[nodiscard]] constexpr auto& get_allocator() const noexcept
        {
            return alloc_adaptor_.get_allocator();
        }

        box() = default;

    private:
        constexpr void deallocate() noexcept
        {
            if(allocation_traits::empty(get_allocation())) return;
            allocation_traits::deallocate(get_allocator(), get_allocations_view());
        }

    public:
        box(const box&)
            requires false;
        box(box&&)
            requires false;

        template<lifetime_req OtherReq>
        explicit(OtherReq != Req) constexpr box(const box<OtherReq, Alloc>& other)
            requires allocation_constructible<
                         allocator_type,
                         callocations_type,
                         allocations_type&,
                         const typename box<OtherReq, allocator_type>::allocation_value&> &&
                         details::box_compatible<Req, allocator_type, OtherReq>
            :
            alloc_adaptor_(other.get_allocator()),
            allocations_{
                allocation_traits::template allocate<allocation_type>(get_allocator(), other.size())
            }
        {
            copy_construct(*this, other);
        }

        template<lifetime_req OtherReq>
        explicit(OtherReq != Req) constexpr box(
            const box<OtherReq, Alloc>& other,
            const allocator_type& alloc //
        )
            requires allocation_constructible<
                         allocator_type,
                         callocations_type,
                         allocations_type&,
                         const typename box<OtherReq, allocator_type>::allocation_value&> &&
                         details::box_compatible<Req, allocator_type, OtherReq>
            :
            alloc_adaptor_(std::in_place, alloc),
            allocations_{
                allocation_traits::template allocate<allocation_type>(get_allocator(), other.size())
            }
        {
            copy_construct(*this, other);
        }

        template<lifetime_req OtherReq>
            requires details::box_compatible<Req, allocator_type, OtherReq>
        explicit(OtherReq != Req) constexpr box(box<OtherReq, allocator_type>&& other) noexcept:
            alloc_adaptor_(cpp_move(other.get_allocator())),
            allocation_value_(other.allocation_value_)
        {
        }

        template<lifetime_req OtherReq>
            requires details::box_compatible<Req, allocator_type, OtherReq>
        explicit(OtherReq != Req) constexpr box(
            box<OtherReq, allocator_type>&& other,
            const allocator_type& alloc //
        ) noexcept:
            alloc_adaptor_(std::in_place, alloc), allocation_value_(other.allocation_value_)
        {
        }

        constexpr void reset() noexcept( //
            allocation_nothrow_destructible<allocator_type, allocations_type&, const allocation_value&>
        )
            requires allocation_destructible<allocator_type, allocations_type&, const allocation_value&>
        {
            allocation_traits::on_destroy(
                get_allocator(),
                get_allocations_view(),
                allocation_value_
            );
            allocation_value_ = {};
        }

    private:
        constexpr void allocate(const std::size_t size)
        {
            get_allocation() =
                allocation_traits::template allocate<allocation_type>(get_allocator(), size);
        }

        template<lifetime_req OtherReq>
        static constexpr void copy_construct(box& instance, const box<OtherReq, Alloc>& other)
        {
            allocation_traits::on_construct(
                instance.get_allocator(),
                other.get_allocations_view(),
                instance.get_allocations_view(),
                other.allocation_value_
            );
        }

        static constexpr void copy_assign(box& instance, const box& other)
        {
            allocation_traits::on_assign(
                other.get_allocations_view(),
                instance.get_allocations_view(),
                other.allocation_value_
            );
        }

        struct cp_assign_fn
        {
            std::reference_wrapper<box> instance;
            std::reference_wrapper<const box> other;

            constexpr void operator()(
                const allocator_propagation<> /*unused*/,
                allocator_type& /*unused*/
            ) const
                requires requires {
                    requires allocation_assignable<
                        allocator_type,
                        callocations_type,
                        allocations_type&,
                        allocation_value>;
                    requires allocation_constructible<
                        allocator_type,
                        callocations_type,
                        allocations_type&,
                        allocation_value>;
                }
            {
                box& instance_ref = instance.get();
                const box& other_ref = other.get();

                if(instance_ref.allocation_value_ == other_ref.allocation_value_)
                {
                    copy_assign(instance_ref, other_ref);
                    return;
                }

                instance_ref.reset();

                if(const auto size = other_ref.size(); instance_ref.capacity() < size)
                {
                    instance_ref.deallocate();
                    instance_ref.allocate(size);
                }

                copy_construct(instance_ref, other_ref);
            }

            constexpr void operator()(
                const allocator_propagation<true> propagation,
                allocator_type& /*unused*/
            ) const
                requires requires { (*this)(allocator_propagation<>{}); }
            {
                if(propagation.assigned) (*this)(allocator_propagation<>{});
            }

            constexpr void operator()(
                const allocator_propagation<false> propagation,
                allocator_type& /*unused*/
            ) const
                requires requires {
                    requires allocation_constructible<
                        allocator_type,
                        callocations_type,
                        allocations_type&,
                        allocation_value>;
                }
            {
                box& instance_ref = instance.get();
                const box& other_ref = other.get();

                if(propagation.assigned)
                {
                    instance_ref.allocate(other_ref.size());
                    copy_construct(instance_ref, other_ref);
                }
                else
                {
                    instance_ref.reset();
                    instance_ref.deallocate();
                }
            }
        };

        static constexpr void move_allocation(box& instance, box& other)
        {
            instance.get_allocation() =
                std::exchange(other.get_allocation(), empty_allocation_result<allocator_type>);
        }

        static constexpr void move_construct(box& instance, box& other)
        {
            allocation_traits::on_construct(
                instance.get_allocator(),
                other.get_allocations_view(),
                instance.get_allocations_view(),
                other.allocation_value_
            );
        }

        static constexpr void move_assign(box& instance, box& other)
        {
            allocation_traits::on_assign(
                other.get_allocations_view(),
                instance.get_allocations_view(),
                other.allocation_value_
            );
        }

        struct mov_assign_fn
        {
            std::reference_wrapper<box> instance;
            std::reference_wrapper<box> other;

            template<bool IsEqual>
            constexpr void operator()(
                const allocator_propagation<IsEqual> propagation,
                allocator_type& /*unused*/
            ) const
                noexcept(allocation_nothrow_destructible<
                         allocator_type,
                         allocations_type&,
                         allocation_value>)
            {
                box& instance_ref = instance.get();
                box& other_ref = other.get();

                if(propagation.assigned)
                {
                    move_allocation(instance_ref, other_ref);
                    return;
                }

                instance_ref.reset();
                instance_ref.deallocate();
            }

            constexpr void operator()(
                const allocator_propagation<> /*unused*/,
                allocator_type& /*unused*/
            ) const
                noexcept(allocation_nothrow_destructible<
                         allocator_type,
                         allocations_type&,
                         allocation_value>)
                requires allocator_traits::always_equal_v
            {
                box& instance_ref = instance.get();
                box& other_ref = other.get();

                instance_ref.reset();
                instance_ref.deallocate();
                move_allocation(instance_ref, other_ref);
            }

            constexpr void operator()(
                const allocator_propagation<> /*unused*/,
                allocator_type& /*unused*/
            ) const
                requires requires {
                    requires allocation_assignable<
                        allocator_type,
                        allocations_type&,
                        allocations_type&,
                        allocation_value>;
                    requires allocation_constructible<
                        allocator_type,
                        allocations_type&,
                        allocations_type&,
                        allocation_value>;
                }
            {
                box& instance_ref = instance.get();
                box& other_ref = other.get();

                if(instance_ref.get_allocator() == other_ref.get_allocator())
                {
                    instance_ref.reset();
                    instance_ref.deallocate();
                    move_allocation(instance_ref, other_ref);
                    return;
                }

                if(instance_ref.allocation_value_ == other_ref.allocation_value_)
                {
                    move_assign(instance_ref, other_ref);
                    return;
                }

                instance_ref.reset();

                if(const auto size = other_ref.size(); instance_ref.capacity() < size)
                {
                    instance_ref.deallocate();
                    instance_ref.allocate(size);
                }

                move_construct(instance_ref, other_ref);
            }
        };

    public:
        constexpr box& operator=(const box& other) //
            noexcept(allocator_nothrow_copy_assignable<allocator_type, cp_assign_fn>)
            requires allocator_copy_assignable<allocator_type, cp_assign_fn>
        {
            alloc_adaptor_.assign(other.get_allocator(), cp_assign_fn{*this, other});
            allocation_value_ = other.allocation_value_;
            return *this;
        }

        constexpr box& operator=(box&& other) //
            noexcept(allocator_nothrow_move_assignable<allocator_type, mov_assign_fn>)
            requires allocator_move_assignable<allocator_type, mov_assign_fn>
        {
            alloc_adaptor_.assign(cpp_move(other.get_allocator()), mov_assign_fn{*this, other});
            allocation_value_ = other.allocation_value_;
            return *this;
        }

        constexpr ~box() noexcept(noexcept(reset()))
            requires requires { reset(); }
        {
            reset();
            deallocate();
        }

    private:
        template<typename T, typename... Args>
        constexpr decltype(auto) emplace_impl(Args&&... args)
            requires requires {
                requires std::constructible_from<allocation_value, std::in_place_type_t<T>>;
                reset();
                requires std::invocable<
                    typename allocation_traits::template constructor<T>,
                    allocator_type&,
                    const allocation_type&,
                    Args...>;
            }
        {
            reset();

            if(const auto size = sizeof(T); capacity() < size)
            {
                deallocate();
                allocate(size);
            }

            allocation_traits::template construct<T>(
                get_allocator(),
                get_allocation(),
                cpp_forward(args)...
            );

            allocation_value_ = allocation_value{std::in_place_type_t<T>{}};
            return get<T>();
        }

    public:
        template<typename T>
        constexpr decltype(auto) emplace(auto&&... args)
            requires requires { emplace_impl<T>(cpp_forward(args)...); }
        {
            emplace_impl<T>(cpp_forward(args)...);
        }

        template<typename T, typename U, typename... Args>
        constexpr decltype(auto) emplace(const std::initializer_list<U> il, Args&&... args)
            requires requires { emplace_impl<T>(il, cpp_forward(args)...); }
        {
            return emplace_impl<T>(il, cpp_forward(args)...);
        }

        template<typename T>
        constexpr decltype(auto) emplace(T&& t)
            requires requires { emplace_impl<std::decay_t<T>>(cpp_forward(t)); }
        {
            return emplace_impl<std::decay_t<T>>(cpp_forward(t));
        }

        template<typename T>
        constexpr box(
            const allocator_type& alloc,
            const std::in_place_type_t<T> /*unused*/,
            auto&&... args
        )
            requires requires { emplace<T>(cpp_forward(args)...); }
            : alloc_adaptor_(std::in_place, alloc)
        {
            emplace<T>(cpp_forward(args)...);
        }

        template<typename T>
        constexpr box(const std::in_place_type_t<T> tag, auto&&... args)
            requires requires { box({}, tag, cpp_forward(args)...); }
            : box({}, tag, cpp_forward(args)...)
        {
        }

        constexpr box(const allocator_type& alloc) noexcept: alloc_adaptor_(std::in_place, alloc) {}

        template<typename T>
        [[nodiscard]] constexpr T& get() noexcept
        {
            return allocation_traits::template get<T>(get_allocation());
        }

        template<typename T>
        [[nodiscard]] constexpr const T& get() const noexcept
        {
            return allocation_traits::template cget<T>(get_allocation());
        }

        [[nodiscard]] constexpr bool has_value() const noexcept
        {
            return allocation_value_.value_size() != 0;
        }

        template<typename T>
        [[nodiscard]] constexpr auto is_type() const noexcept
        {
            if constexpr(std::constructible_from<allocation_value, std::in_place_type_t<T>>)
                return allocation_value_ == allocation_value{std::in_place_type_t<T>{}};
            else return false;
        }

        [[nodiscard]] constexpr auto size() const noexcept
        {
            return allocation_value_.value_size();
        }

        [[nodiscard]] constexpr auto capacity() const noexcept { return get_allocation().size(); }
    }; // NOLINTEND(*-noexcept-*)

    template<typename T, typename Alloc>
    box(const Alloc&, std::in_place_type_t<T>, auto&&...)
        -> box<lifetime_req::for_type<T>(), Alloc>;

    template<typename T, allocator_req Alloc>
    using box_for = box<lifetime_req::for_type<T>(), Alloc>;

    template<allocator_req Alloc>
    struct make_box_fn
    {
        template<typename T>
            requires std::constructible_from<box_for<std::decay_t<T>, Alloc>, T>
        [[nodiscard]] constexpr box_for<std::decay_t<T>, Alloc> operator()(T&& t) const
        {
            using decay_t = std::decay_t<T>;
            return {std::in_place_type<decay_t>, cpp_forward(t)};
        }
    };

    template<allocator_req Alloc>
    inline constexpr make_box_fn<Alloc> make_box{};

    template<allocator_req Alloc>
    using trivial_box = box_for<trivial_object, Alloc>;

    template<allocator_req Alloc>
    using normal_box = box_for<normal_object, Alloc>;

    template<allocator_req Alloc>
    using unique_box = box_for<unique_object, Alloc>;
}
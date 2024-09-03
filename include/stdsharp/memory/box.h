#pragma once

#include "../utility/dispatcher.h"
#include "allocation_adaptor.h"
#include "allocator_adaptor.h"
#include "stdsharp/concepts/type.h"

namespace stdsharp::details
{
    template<lifetime_req Req, allocator_req Alloc>
    struct box_traits
    {
        using allocation_traits = allocation_traits<Alloc>;

        using allocator_type = allocation_traits::allocator_type;

        using allocator_traits = allocation_traits::allocator_traits;

        using allocation_type = allocation_traits::allocation;

        using callocation_type = allocation_traits::callocation;

        using fake_type = fake_type<Req>;

        using alloc_adaptor = allocator_adaptor<allocator_type>;

        static constexpr auto req = allocator_traits::template type_req<fake_type>;

        using dispatchers = stdsharp::invocables<
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
            dispatcher<expr_req::no_exception, void, allocator_type&, const allocation_type&>>;

        using allocation_adaptor = stdsharp::allocation_adaptor<allocator_type, box_traits>;

        using allocations_type = std::array<allocation_type, 1>;
        using callocations_type =
            cast_view<std::ranges::ref_view<const allocations_type>, callocation_type>;
    };

    template<lifetime_req Req, typename Alloc, typename T>
    concept box_type_compatible = std::constructible_from<
        typename box_traits<Req, Alloc>::dispatchers,
        allocation_adaptor<Alloc, T>,
        allocation_adaptor<Alloc, T>,
        allocation_adaptor<Alloc, T>,
        allocation_adaptor<Alloc, T>,
        allocation_adaptor<Alloc, T>>;

    template<lifetime_req Req, typename Alloc, lifetime_req OtherReq>
    concept box_compatible =
        box_type_compatible<Req, Alloc, typename box_traits<Req, Alloc>::fake_type>;

    template<
        lifetime_req Req,
        typename Alloc,
        lifetime_req OtherReq,
        typename Traits = box_traits<Req, Alloc>>
    concept box_copy_constructible = box_compatible<Req, Alloc, OtherReq> &&
        allocation_constructible<Alloc,
                                 typename Traits::callocations_type,
                                 typename Traits::allocations_type,
                                 const typename Traits::allocation_adaptor&>;

    template<
        lifetime_req Req,
        typename Alloc,
        lifetime_req OtherReq,
        typename Traits = box_traits<Req, Alloc>>
    concept box_move_constructible = box_compatible<Req, Alloc, OtherReq> &&
        allocation_constructible<Alloc,
                                 typename Traits::allocations_type,
                                 typename Traits::allocations_type,
                                 const typename Traits::allocation_adaptor&>;
}

namespace stdsharp
{
    template<typename Alloc, lifetime_req Req>
    class allocation_adaptor<Alloc, details::box_traits<Req, Alloc>>
    {
        using m_dispatchers = details::box_traits<Req, Alloc>::dispatchers;

        indexed_values<m_dispatchers, std::size_t> values_{};
        cttid_t type_;

        constexpr decltype(auto) dispatchers(this auto&& self) noexcept
        {
            return cpp_forward(self).values_.template get<0>();
        }

        static constexpr auto self_cast = fwd_cast<allocation_adaptor>;

    public:
        allocation_adaptor() = default;

        template<
            typename Self,
            typename... Args,
            std::invocable<Args...> Dispatchers = forward_like_t<Self, m_dispatchers>>
        constexpr void operator()(this Self&& self, Args&&... args)
            noexcept(nothrow_invocable<Dispatchers, Args...>)
        {
            self_cast(cpp_forward(self)).dispatchers()(cpp_forward(args)...);
        }

        [[nodiscard]] auto& type() const noexcept { return type_; }

        constexpr bool operator==(const allocation_adaptor& other) const noexcept
        {
            return type_ == other.type_;
        }

        template<typename T, typename Op = allocation_adaptor<Alloc, T>>
            requires details::box_type_compatible<Req, Alloc, T>
        explicit constexpr allocation_adaptor(const std::in_place_type_t<T> /*unused*/) noexcept:
            values_(m_dispatchers{Op{}, Op{}, Op{}, Op{}, Op{}}, std::size_t{sizeof(T)}),
            type_(cttid<T>)
        {
        }

        template<lifetime_req OtherReq>
            requires details::box_compatible<Req, Alloc, OtherReq> && (Req != OtherReq)
        explicit constexpr allocation_adaptor(
            const allocation_adaptor<Alloc, details::box_traits<OtherReq, Alloc>> other
        ) noexcept:
            values_(m_dispatchers{other, other, other, other, other}, {other.value_size_()}),
            type_(other.type_)
        {
        }

        [[nodiscard]] constexpr auto value_size() const noexcept
        {
            return values_.template get<1>();
        }
    };
}

namespace stdsharp::details
{
    template<lifetime_req Req, typename Alloc, typename Traits = details::box_traits<Req, Alloc>>
    class box : Traits::alloc_adaptor
    {
        using allocation_traits = Traits::allocation_traits;
        using allocator_traits = Traits::allocator_traits;
        using allocation_type = Traits::allocation_type;
        using callocation_type = Traits::callocation_type;
        using allocation_adaptor = Traits::allocation_adaptor;
        using allocations_type = Traits::allocations_type;
        using callocations_type = Traits::callocations_type;
        using alloc_adaptor = Traits::alloc_adaptor;

        template<lifetime_req, typename>
        friend class box;

        friend class Traits::alloc_adaptor;

        allocations_type allocations_;
        allocation_adaptor allocation_adaptor_{};

    public:
        using typename Traits::allocator_type;

    private:
        [[nodiscard]] constexpr auto& get_allocations() noexcept { return allocations_; }

        [[nodiscard]] constexpr auto get_allocations() const noexcept
        {
            return callocations_type{std::ranges::ref_view{allocations_}, {}};
        }

        [[nodiscard]] constexpr auto& get_allocation() noexcept { return allocations_.front(); }

        [[nodiscard]] constexpr auto& get_allocation() const noexcept
        {
            return allocations_.front();
        }

        [[nodiscard]] constexpr Alloc& get_allocator() noexcept { return *this; }

    public:
        [[nodiscard]] constexpr const Alloc& get_allocator() const noexcept { return *this; }

        box() = default;

    private:
        constexpr void allocate(const auto size) noexcept
        {
            return allocation_traits::template allocate<allocation_type>(get_allocator(), size);
        }

        constexpr void deallocate() noexcept
        {
            if(allocation_traits::empty(get_allocation())) return;
            allocation_traits::deallocate(get_allocator(), get_allocations());
        }

        constexpr void reset() noexcept
        {
            allocation_traits::on_destroy(get_allocator(), get_allocations(), allocation_adaptor_);
            allocation_adaptor_ = {};
        }

        template<lifetime_req OtherReq>
        constexpr void construct_by(box<OtherReq, Alloc>& other)
        {
            allocation_traits::on_construct(
                get_allocator(),
                other.get_allocations(),
                get_allocations(),
                other.allocation_adaptor_
            );
        }

        template<lifetime_req OtherReq>
        constexpr void construct_by(const box<OtherReq, Alloc>& other)
        {
            allocation_traits::on_construct(
                get_allocator(),
                other.get_allocations(),
                get_allocations(),
                other.allocation_adaptor_
            );
        }

    public:
        box(const box&)
            requires false;
        box(box&&) noexcept
            requires false;

        template<lifetime_req OtherReq>
            requires box_copy_constructible<Req, allocator_type, OtherReq>
        explicit(OtherReq != Req) constexpr box(const box<OtherReq, Alloc>& other):
            alloc_adaptor(other), allocations_(allocate(other.size()))
        {
            construct_by(other);
        }

        template<lifetime_req OtherReq>
            requires box_copy_constructible<Req, allocator_type, OtherReq>
        explicit(
            OtherReq != Req
        ) constexpr box(const box<OtherReq, Alloc>& other, const allocator_type& alloc):
            alloc_adaptor(alloc), allocations_(allocate(other.size()))
        {
            construct_by(other);
        }

        template<lifetime_req OtherReq>
            requires details::box_compatible<Req, allocator_type, OtherReq>
        explicit(OtherReq != Req) constexpr box(box<OtherReq, allocator_type>&& other) noexcept:
            alloc_adaptor(cpp_move(other)),
            allocations_(cpp_move(other.allocations_)),
            allocation_adaptor_(cpp_move(other.allocation_adaptor_))
        {
        }

        template<lifetime_req OtherReq>
            requires details::box_compatible<Req, allocator_type, OtherReq> &&
                         allocator_traits::always_equal_v
        explicit(OtherReq != Req) constexpr box(
            box<OtherReq, allocator_type>&& other,
            const allocator_type& alloc //
        ) noexcept:
            alloc_adaptor(std::in_place, alloc),
            allocations_(cpp_move(other.allocations_)),
            allocation_adaptor_(cpp_move(other.allocation_adaptor_))
        {
        }

        template<lifetime_req OtherReq>
            requires details::box_compatible<Req, allocator_type, OtherReq>
        explicit(OtherReq != Req) constexpr box(
            box<OtherReq, allocator_type>&& other,
            const allocator_type& alloc //
        ) noexcept:
            alloc_adaptor(std::in_place, alloc), allocation_adaptor_(other.allocation_adaptor_)
        {
            if(alloc == other) allocations_ = other.allocations_;
            else
            {
                allocations_ = allocate(other.size());
                construct_by(other);
            }
        }

        template<typename T>
        constexpr box(
            const allocator_type& alloc,
            const std::in_place_type_t<T> /*unused*/,
            auto&&... args
        )
            requires requires { emplace<T>(cpp_forward(args)...); }
            : alloc_adaptor(std::in_place, alloc)
        {
            emplace<T>(cpp_forward(args)...);
        }

        template<typename T>
        constexpr box(const std::in_place_type_t<T> /*unused*/, auto&&... args)
            requires requires { emplace<T>(cpp_forward(args)...); }
        {
            emplace<T>(cpp_forward(args)...);
        }

        constexpr box(const allocator_type& alloc) noexcept: alloc_adaptor(alloc) {}

    private:
        constexpr void assign_impl(auto& other)
        {
            const auto& other_adaptor = other.allocation_adaptor_;

            if(allocation_adaptor_ == other_adaptor)
            {
                allocation_traits::
                    on_assign(other.get_allocations_view(), get_allocations(), other_adaptor);
                return;
            }

            reset();

            if(const auto size = other.size(); capacity() < size)
            {
                deallocate();
                allocate(size);
            }

            construct_by(other);
        }

        template<typename T>
            requires same_as_any<
                T,
                typename alloc_adaptor::no_swap_propagation,
                typename alloc_adaptor::template swap_propagation<true>,
                typename alloc_adaptor::template swap_propagation<false>>
        constexpr void operator()(const T /*unused*/, box& rhs) noexcept
        {
            std::ranges::swap(allocations_, rhs.allocations_);
            std::ranges::swap(allocation_adaptor_, rhs.allocation_adaptor_);
        }

        constexpr void operator()(const alloc_adaptor::no_copy_propagation /*unused*/) const
            requires requires {
                requires allocation_assignable<
                    allocator_type,
                    callocations_type,
                    allocations_type&,
                    allocation_adaptor>;
                requires allocation_constructible<
                    allocator_type,
                    callocations_type,
                    allocations_type&,
                    allocation_adaptor>;
            }
        {
            assign_impl(instance, other);
        }

        constexpr void operator()(
            const allocator_propagation_t<true, false> /*unused*/,
            allocator_type& /*unused*/
        ) const
            requires requires { (*this)(); }
        {
            (*this)();
        }

        constexpr void operator()(
            const allocator_propagation_t<false, true> /*unused*/,
            allocator_type& /*unused*/
        ) const noexcept
        {
            box& instance_ref = instance.get();

            instance_ref.reset();
            instance_ref.deallocate();
        }

        constexpr void operator()(
            const allocator_propagation_t<false, false> /*unused*/,
            allocator_type& /*unused*/
        ) const
            requires allocation_constructible<
                allocator_type,
                callocations_type,
                allocations_type&,
                allocation_adaptor>
        {
            box& instance_ref = instance.get();
            const box& other_ref = other.get();

            instance_ref.allocate(other_ref.size());
            construct_by(instance_ref, other_ref);
        }

        struct cp_assign_fn : empty_t
        {
            std::reference_wrapper<box> instance;
            std::reference_wrapper<const box> other;
        };

        struct mov_assign_fn
        {
            std::reference_wrapper<box> instance;
            std::reference_wrapper<box> other;

        private:
            constexpr void reset()
            {
                box& instance_ref = instance.get();
                instance_ref.reset();
                instance_ref.deallocate();
            }

            constexpr void move_allocation()
            {
                instance.get().get_allocation() = std::exchange(
                    other.get().get_allocation(),
                    empty_allocation<allocator_type> //
                );
            }

            constexpr void move_impl() const noexcept
            {
                reset();
                move_allocation();
            }

        public:
            template<bool IsEqual>
            constexpr void operator()(
                const allocator_propagation_t<IsEqual, true> /*unused*/,
                allocator_type& /*unused*/
            ) const noexcept
            {
                reset();
            }

            template<bool IsEqual>
            constexpr void operator()(
                const allocator_propagation_t<IsEqual, false> /*unused*/,
                allocator_type& /*unused*/
            ) const noexcept
            {
                move_allocation();
            }

            constexpr void operator()(allocator_type& /*unused*/) const noexcept
                requires allocator_traits::always_equal_v
            {
                move_impl();
            }

            constexpr void operator()(allocator_type& /*unused*/) const
                requires requires {
                    requires allocation_assignable<
                        allocator_type,
                        allocations_type&,
                        allocations_type&,
                        allocation_adaptor>;
                    requires allocation_constructible<
                        allocator_type,
                        allocations_type&,
                        allocations_type&,
                        allocation_adaptor>;
                }
            {
                box& instance_ref = instance.get();
                box& other_ref = other.get();

                if(instance_ref.get_allocator() == other_ref.get_allocator())
                {
                    move_impl();
                    return;
                }

                assign_impl(instance_ref, other_ref);
            }
        };

    public:
        constexpr box& operator=(const box& other)
            noexcept(allocator_nothrow_copy_assignable<allocator_type, cp_assign_fn>)
            requires allocator_copy_assignable<allocator_type, cp_assign_fn>
        {
            alloc_adaptor_.assign(other.get_allocator(), cp_assign_fn{*this, other});
            allocation_adaptor_ = other.allocation_adaptor_;
            return *this;
        }

        constexpr box& operator=(box&& other)
            noexcept(allocator_nothrow_move_assignable<allocator_type, mov_assign_fn>)
            requires allocator_move_assignable<allocator_type, mov_assign_fn>
        {
            alloc_adaptor_.assign(cpp_move(other.get_allocator()), mov_assign_fn{*this, other});
            allocation_adaptor_ = other.allocation_adaptor_;
            return *this;
        }

        constexpr ~box()
        {
            reset();
            deallocate();
        }

        template<typename T, typename... Args>
        constexpr decltype(auto) emplace_impl(Args&&... args)
            requires requires {
                requires std::constructible_from<allocation_adaptor, std::in_place_type_t<T>>;
                requires std::invocable<
                    typename allocation_traits::template constructor<T>,
                    allocator_type&,
                    const allocation_type&,
                    Args...>;
            }
        {
            reset();

            if(constexpr auto size = sizeof(T); capacity() < size)
            {
                deallocate();
                get_allocation() =
                    allocation_traits::template allocate<allocation_type>(get_allocator(), size);
            }

            allocation_traits::
                template construct<T>(get_allocator(), get_allocation(), cpp_forward(args)...);

            allocation_adaptor_ = allocation_adaptor{std::in_place_type_t<T>{}};
            return get<T>();
        }

    public:
        template<typename T, typename... Args>
        constexpr T& emplace(Args&&... args)
            requires requires { this->emplace_impl<T>(cpp_forward(args)...); }
        {
            return emplace_impl<T>(cpp_forward(args)...);
        }

        template<typename T, typename U, typename... Args>
        constexpr T& emplace(const std::initializer_list<U> il, Args&&... args)
            requires requires { this->emplace_impl<T>(il, cpp_forward(args)...); }
        {
            return emplace_impl<T>(il, cpp_forward(args)...);
        }

        template<typename T>
        constexpr T& emplace(T&& t)
            requires requires { this->emplace_impl<std::decay_t<T>>(cpp_forward(t)); }
        {
            return emplace_impl<std::decay_t<T>>(cpp_forward(t));
        }

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
            return allocation_adaptor_.value_size() != 0;
        }

        template<typename T>
        [[nodiscard]] constexpr auto is_type() const noexcept
        {
            if constexpr(std::constructible_from<allocation_adaptor, std::in_place_type_t<T>>)
                return allocation_adaptor_.type() == cttid<T>;
            else return false;
        }

        [[nodiscard]] constexpr auto size() const noexcept
        {
            return allocation_adaptor_.value_size();
        }

        [[nodiscard]] constexpr auto capacity() const noexcept { return get_allocation().size(); }
    }; // NOLINTEND(*-noexcept-*)
}

namespace stdsharp
{
    template<lifetime_req Req, allocator_req Alloc>
    class box : details::box<Req, Alloc>
    {
    };

    template<typename T, typename Alloc>
    box(const Alloc&, std::in_place_type_t<T>, auto&&...
    ) -> box<lifetime_req::for_type<T>(), Alloc>;

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

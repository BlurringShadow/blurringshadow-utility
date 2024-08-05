#pragma once

#include "../functional/nodiscard_invocable.h"
#include "../functional/sequenced_invocables.h"
#include "../utility/auto_cast.h"

#include <memory>

#include "../compilation_config_in.h"

namespace stdsharp::details
{
    template<typename Ptr>
    struct pointer_traits
    {
    private:
        using std_traits = std::pointer_traits<Ptr>;

        struct no_type
        {
        };

        static consteval auto get_pointer_type()
        {
            if constexpr(requires { typename std_traits::pointer; })
                return type_constant<typename std_traits::pointer>{};
            else return type_constant<Ptr>{};
        }

        static consteval auto get_element_type()
        {
            if constexpr(requires { typename std_traits::element_type; })
                return type_constant<typename std_traits::element_type>{};
            else if constexpr(dereferenceable<Ptr>)
                return type_constant<std::remove_reference_t<decltype(*std::declval<Ptr>())>>{};
            else return no_type{};
        }

    public:
        using pointer = typename decltype(get_pointer_type())::type;

        template<typename T = decltype(get_element_type())>
        struct element_type_t
        {
            using element_type = T::type;
        };

        template<>
        struct element_type_t<no_type>
        {
        };

        template<typename U>
        using rebind = decltype( //
            []<typename Base = std_traits>
            {
                return type_constant<typename Base::template rebind<U>>{}; //
            }()
        );

    private:
        struct traits_to_address
        {
            void pointer_test(auto*);

            [[nodiscard]] constexpr decltype(auto) operator()(const pointer& p) const noexcept
                requires requires { std_traits::to_address(p); }
            {
                return std_traits::to_address(p);
            }
        };

        struct cast_to_address
        {
            static constexpr auto cast_to_pointer(auto* p) noexcept { return p; }

            [[nodiscard]] constexpr decltype(auto) operator()(const pointer& p) const noexcept
                requires requires {
                    cast_to_pointer(p);
                    requires !function<decltype(cast_to_pointer(p))>;
                }
            {
                return std::to_address(cast_to_pointer(p));
            }
        };

        struct operator_to_address
        {
            [[nodiscard]] constexpr decltype(auto) operator()(const pointer& p) const noexcept
                requires requires { std::to_address(p.operator->()); }
            {
                return std::to_address(p.operator->());
            }
        };

        struct convert_to_address
        {
            [[nodiscard]] constexpr decltype(auto) operator()(const pointer& p) const noexcept
                requires requires { std::addressof(*p); }
            {
                return std::addressof(*p);
            }
        };

    public:
        using to_address_fn = nodiscard_invocable<stdsharp::sequenced_invocables<
            traits_to_address,
            cast_to_address,
            operator_to_address,
            convert_to_address>>;

        static constexpr to_address_fn to_address{};

    private:
        template<typename Element = decltype(get_element_type())::type>
        static consteval auto get_raw_pointer()
        {
            if constexpr(std::invocable<to_address_fn, pointer>)
                return static_cast<std::invoke_result_t<to_address_fn, pointer>>(nullptr);
            else if constexpr(std::same_as<const void, Element>)
                return static_cast<const void*>(nullptr);
            else if constexpr(std::same_as<void, Element>)
            {
                if constexpr(requires { pointer{static_cast<const void*>(nullptr)}; })
                    return static_cast<const void*>(nullptr);
                else if constexpr(requires { pointer{static_cast<void*>(nullptr)}; })
                    return static_cast<void*>(nullptr);
                else return no_type{};
            }
            else return no_type{};
        }

    public:
        template<typename RawPtr = decltype(get_raw_pointer())>
        struct raw_pointer_t
        {
            using raw_pointer = RawPtr;
        };

        template<>
        struct raw_pointer_t<no_type>
        {
        };

    private:
        static consteval auto get_difference_type()
        {
            if constexpr(requires { typename std_traits::difference_type; })
                return type_constant<typename std_traits::difference_type>{};
            else if constexpr(requires(pointer p) { p - p; })
                return type_constant<decltype(std::declval<pointer&>() - std::declval<pointer&>())>{
                };
            else if constexpr(requires(raw_pointer_t<>::raw_pointer p) { p - p; })
            {
                using raw_pointer = raw_pointer_t<>::raw_pointer;

                return type_constant<
                    decltype(std::declval<raw_pointer&>() - std::declval<raw_pointer&>())>{};
            }
            else return no_type{};
        }

    public:
        template<typename T = decltype(get_difference_type())>
        struct difference_type_t
        {
            using difference_type = T::type;
        };

        template<>
        struct difference_type_t<no_type>
        {
        };

        template<
            typename = typename decltype(get_element_type())::type,
            typename = decltype(get_raw_pointer())>
        struct pointer_to_fn_t
        {
        };

        template<
            std::same_as<typename element_type_t<>::element_type> Element,
            std::same_as<typename raw_pointer_t<>::raw_pointer> RawPtr>
            requires(!void_<Element>)
        struct pointer_to_fn_t<Element, RawPtr>
        {
        private:
            struct base_pointer_to
            {
                [[nodiscard]] constexpr pointer operator()(Element& t) const
                    noexcept(noexcept(Ptr::pointer_to(t)))
                    requires requires { Ptr::pointer_to(t); }
                {
                    return std_traits::pointer_to(t);
                }
            };

            struct convert_pointer_to
            {
                [[nodiscard]] constexpr pointer operator()(Element& r) const
                    noexcept(nothrow_explicitly_convertible<RawPtr, pointer>)
                    requires explicitly_convertible<RawPtr, pointer>
                {
                    return auto_cast(std::addressof(r));
                }
            };

        public:
            using pointer_to_fn =
                nodiscard_invocable<stdsharp::
                                        sequenced_invocables<base_pointer_to, convert_pointer_to>>;

            static constexpr pointer_to_fn pointer_to{};
        };

        template<typename = decltype(get_raw_pointer())>
        struct to_pointer_fn_t
        {
        };

        template<std::same_as<typename raw_pointer_t<>::raw_pointer> RawPtr>
        struct to_pointer_fn_t<RawPtr>
        {
        private:
            struct dereference_to_pointer
            {
                [[nodiscard]] constexpr pointer operator()(const RawPtr p) const
                    noexcept(noexcept(pointer_to_fn_t<>::pointer_to(*p), pointer{}))
                    requires requires {
                        requires nullable_pointer<pointer>;
                        pointer_to_fn_t<>::pointer_to(*p);
                    }
                {
                    return p == nullptr ? pointer{} : pointer_to_fn_t<>::pointer_to(*p);
                }
            };

            struct convert_to_pointer
            {
                STDSHARP_INTRINSIC constexpr pointer operator()(const RawPtr p) const
                    noexcept(nothrow_explicitly_convertible<RawPtr, pointer>)
                    requires explicitly_convertible<RawPtr, pointer>
                {
                    return static_cast<pointer>(p);
                }
            };

        public:
            using to_pointer_fn = nodiscard_invocable<
                stdsharp::sequenced_invocables<dereference_to_pointer, convert_to_pointer>>;

            static constexpr to_pointer_fn to_pointer{};
        };
    };
}

namespace stdsharp
{
    template<typename Ptr>
    struct pointer_traits :
        details::pointer_traits<Ptr>,
        details::pointer_traits<Ptr>::template element_type_t<>,
        details::pointer_traits<Ptr>::template difference_type_t<>,
        details::pointer_traits<Ptr>::template raw_pointer_t<>,
        details::pointer_traits<Ptr>::template pointer_to_fn_t<>,
        details::pointer_traits<Ptr>::template to_pointer_fn_t<>
    {
    };

    inline constexpr struct to_void_pointer_fn
    {
        template<typename T>
            requires std::invocable<typename pointer_traits<T>::to_address_fn, const T&>
        [[nodiscard]] constexpr auto operator()(const T& ptr) const noexcept
        {
            return (*this)(pointer_traits<T>::to_address(ptr));
        }

        template<typename T>
        STDSHARP_INTRINSIC constexpr auto operator()(const T* const ptr) const noexcept
        {
            return static_cast<const void*>(ptr);
        }

        template<typename T>
        STDSHARP_INTRINSIC constexpr auto operator()(T* const ptr) const noexcept
        {
            return static_cast<void*>(ptr);
        }
    } to_void_pointer{};

    template<typename T>
    struct pointer_cast_fn
    {
    private:
        template<typename Pointer>
            requires requires { pointer_traits<Pointer>{}; }
        struct traits
        {
            static const T* test(const auto*);
            static T* test(auto*);

            using ptr = decltype(test(typename pointer_traits<Pointer>::raw_pointer{}));
        };

    public:
        STDSHARP_INTRINSIC constexpr auto operator()(const void* const ptr) const noexcept
        {
            return static_cast<const T*>(ptr);
        }

        STDSHARP_INTRINSIC constexpr auto operator()(void* const ptr) const noexcept
        {
            return static_cast<T*>(ptr);
        }

        template<typename Pointer>
            requires requires {
                requires std::invocable<to_void_pointer_fn, const Pointer&>;
                requires !nothrow_explicitly_convertible<Pointer, typename traits<Pointer>::ptr>;
            }
        [[nodiscard]] constexpr auto operator()(const Pointer& p) const noexcept
        {
            return (*this)(to_void_pointer(p));
        }

        template<typename Pointer>
            requires nothrow_explicitly_convertible<Pointer, typename traits<Pointer>::ptr>
        STDSHARP_INTRINSIC constexpr auto operator()(const Pointer& p) const noexcept
        {
            return static_cast<typename traits<Pointer>::ptr>(p);
        }
    };

    template<typename T>
    inline constexpr pointer_cast_fn<T> pointer_cast{};

    template<typename T>
    struct launder_cast_fn
    {
        template<typename Pointer>
            requires std::invocable<pointer_cast_fn<T>, const Pointer&>
        [[nodiscard]] constexpr auto operator()(const Pointer& p) const noexcept
        {
            return std::launder(pointer_cast<T>(p));
        }
    };

    template<typename T>
    inline constexpr launder_cast_fn<T> launder_cast{};
}

#include "../compilation_config_out.h"

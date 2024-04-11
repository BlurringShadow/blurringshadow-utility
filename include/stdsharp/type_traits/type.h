#pragma once

#include "../concepts/type.h"
#include "../functional/invoke.h"
#include "../macros.h"
#include "regular_type_sequence.h"

namespace stdsharp
{
    template<typename T, typename U>
    using is_derived_from = std::is_base_of<U, T>;

    enum class ref_qualifier : std::uint8_t
    {
        none,
        rvalue,
        lvalue
    };

    template<typename T>
    inline constexpr auto ref_qualifier_v = lvalue_ref<T> ? //
        ref_qualifier::lvalue :
        rvalue_ref<T> ? ref_qualifier::rvalue : ref_qualifier::none;

    template<typename T, bool Const>
    using apply_const = std::conditional_t<Const, std::add_const_t<T>, T>;

    template<typename T, bool Volatile>
    using apply_volatile = std::conditional_t<Volatile, std::add_volatile_t<T>, T>;

    template<typename T, ref_qualifier ref>
    using apply_ref = std::conditional_t<
        ref == ref_qualifier::lvalue,
        std::add_lvalue_reference_t<T>,
        std::conditional_t<ref == ref_qualifier::rvalue, std::add_rvalue_reference_t<T>, T>>;

    template<typename T, bool Const, bool Volatile, ref_qualifier ref>
    using apply_qualifiers =
        apply_ref<apply_volatile<apply_const<std::remove_cvref_t<T>, Const>, Volatile>, ref>;

    template<typename T>
    using add_const_lvalue_ref_t = std::add_lvalue_reference_t<std::add_const_t<T>>;

    template<typename T, typename U>
    using ref_align_t = apply_ref<U, ref_qualifier_v<T>>;

    template<typename T, typename U>
    using const_align_t = apply_const<U, const_<T>>;

    template<typename T, typename U>
    using volatile_align_t = apply_volatile<U, volatile_<T>>;

    template<typename T, typename U>
    using cv_ref_align_t = ref_align_t<T, volatile_align_t<T, const_align_t<T, U>>>;

    template<typename, ref_qualifier>
    struct override_ref;

    template<typename T>
    struct override_ref<T, ref_qualifier::lvalue>
    {
        using type = std::remove_reference_t<T>&;
    };

    template<typename T>
    struct override_ref<T, ref_qualifier::rvalue>
    {
        using type = std::remove_reference_t<T>&&;
    };

    template<typename T>
    struct override_ref<T, ref_qualifier::none>
    {
        using type = T;
    };

    template<typename T, ref_qualifier ref>
    using override_ref_t = typename override_ref<T, ref>::type;

    template<typename T, typename... U>
    struct ref_collapse
    {
        static constexpr auto value = []
        {
            ref_qualifier value = ref_qualifier::none;
            for(const auto r : {ref_qualifier_v<T>, ref_qualifier_v<U>...})
                if(r > value) value = r;
            return value;
        }();

        using type = override_ref_t<T, value>;
    };

    template<typename T, typename... U>
    inline constexpr auto ref_collapse_v = ref_collapse<T, U...>::value;

    template<typename T, typename... U>
    using ref_collapse_t = typename ref_collapse<T, U...>::type;

    template<typename T, typename... U>
    using const_collapse_t = std::
        conditional_t<(std::is_const_v<T> || ... || std::is_const_v<U>), std::add_const_t<T>, T>;

    template<typename...>
    [[nodiscard]] constexpr auto dependent_false(const auto&... /*unused*/) noexcept
    {
        return false;
    }

    template<template<typename...> typename T, typename... Args>
    struct deduction
    {
        using type = T<std::decay_t<Args>...>;
    };

    template<template<typename...> typename T, typename... Args>
    using deduction_t = typename deduction<T, Args...>::type;

    template<template<typename...> typename T>
    struct make_template_type_fn
    {
        template<typename... Args>
            requires requires { typename deduction<T, Args...>::type; }
        using type = deduction_t<T, Args...>;

        template<typename... Args>
            requires std::constructible_from<type<Args...>, Args...>
        constexpr auto operator()(Args&&... args) const
            noexcept(std::is_nothrow_constructible_v<type<Args...>, Args...>)
        {
            return type<Args...>{cpp_forward(args)...};
        }
    };

    template<template<typename> typename T, typename... Ts>
    struct ttp_expend : T<Ts>...
    {
        ttp_expend() = default;

        template<typename... Us>
            requires(std::constructible_from<T<Ts>, Us> && ...)
        constexpr ttp_expend(Us&&... us)
            noexcept((std::is_nothrow_constructible_v<T<Ts>, Us> && ...)):
            T<Ts>(cpp_forward(us))...
        {
        }
    };

    template<template<typename> typename T, typename... Ts>
    ttp_expend(T<Ts>...) -> ttp_expend<T, Ts...>;

    template<template<typename> typename T, typename Seq>
    using make_ttp_expend_by = decltype( //
        []<template<typename...> typename Inner, typename... U>(const Inner<U...>&)
        {
            return std::type_identity<ttp_expend<T, U...>>{}; //
        }(std::declval<Seq>())
    )::type;

    template<template<auto> typename T, decltype(auto)... V>
    struct nttp_expend : T<V>...
    {
        nttp_expend() = default;

        template<typename... Us>
            requires(std::constructible_from<T<V>, Us> && ...)
        constexpr nttp_expend(Us&&... us)
            noexcept((std::is_nothrow_constructible_v<T<V>, Us> && ...)):
            T<V>(cpp_forward(us))...
        {
        }
    };

    template<template<auto> typename T, decltype(auto)... V>
    nttp_expend(T<V>...) -> nttp_expend<T, V...>;

    template<template<auto> typename T, std::size_t Size>
    using make_nttp_expend = decltype( //
        []<template<auto...> typename Inner, decltype(auto)... V>(const Inner<V...>)
        {
            return std::type_identity<nttp_expend<T, V...>>{}; //
        }(std::make_index_sequence<Size>{})
    );
}

namespace stdsharp::details
{
    template<typename R, bool Noexcept, typename... Args>
    struct function_traits
    {
        static auto constexpr is_noexcept = Noexcept;

        using result_t = R;
        using args_t = regular_type_sequence<Args...>;
        using function_t = R(Args...) noexcept(Noexcept);
        using type = R (*)(Args...) noexcept(Noexcept);
    };

    template<bool, typename, typename, typename...>
    struct mem_func_pointer;

#define MEM_FUNC_POINTER(cv, ref)                                       \
    template<bool Noexcept, typename Ret, typename T, typename... Args> \
    struct mem_func_pointer<Noexcept, Ret, cv T ref, Args...>           \
    {                                                                   \
        using type = Ret (T::*)(Args...) cv ref noexcept(Noexcept);     \
    };

    MEM_FUNC_POINTER(const, &)
    MEM_FUNC_POINTER(const, &&)
    MEM_FUNC_POINTER(volatile, &)
    MEM_FUNC_POINTER(volatile, &&)
    MEM_FUNC_POINTER(const volatile, &)
    MEM_FUNC_POINTER(const volatile, &&)
    MEM_FUNC_POINTER(, &)
    MEM_FUNC_POINTER(, &&)

#undef MEM_FUNC_POINTER

    template<typename... T, template<typename...> typename Inner, typename... U>
    consteval Inner<T...> template_rebind_impl(Inner<U...>);
}

namespace stdsharp
{
    template<typename Template, typename... T>
        requires requires { details::template_rebind_impl<T...>(std::declval<Template>()); }
    using template_rebind = decltype(details::template_rebind_impl<T...>(std::declval<Template>()));

    template<typename>
    struct function_traits;

    template<typename T>
        requires requires { function_traits<std::decay_t<T>>{}; }
    struct function_traits<T> : function_traits<std::decay_t<T>>
    {
    };

    template<typename R, typename... Args>
    struct function_traits<R (*)(Args...)> : details::function_traits<R, false, Args...>
    {
    };

    template<typename R, typename... Args>
    struct function_traits<R (*)(Args...) noexcept> : details::function_traits<R, true, Args...>
    {
    };

    template<auto Ptr>
    using function_pointer_traits = function_traits<decltype(Ptr)>;

    template<bool Noexcept, typename Ret, typename... Args>
    using func_pointer = Ret (*)(Args...) noexcept(Noexcept);

    template<typename>
    struct member_traits;

    template<typename T, typename ClassT>
    struct member_traits<T ClassT::*>
    {
        using type = T;
        using class_t = ClassT;
    };

    template<auto Ptr>
    struct member_pointer_traits : member_traits<decltype(Ptr)>
    {
        static constexpr auto ptr = Ptr;

        constexpr auto operator()(auto&&... args) const
            noexcept(noexcept(invoke(ptr, cpp_forward(args)...))
            ) -> decltype(invoke(ptr, cpp_forward(args)...))
        {
            return invoke(ptr, cpp_forward(args)...);
        }
    };

#define STDSHARP_MEMBER_FUNCTION_TRAITS(const_, volatile_, ref_, qualifiers)                  \
    template<typename R, typename ClassT, typename... Args, bool Noexcept>                    \
    struct member_traits<R (ClassT::*)(Args...) qualifiers noexcept(Noexcept)> :              \
        function_traits<R (*)(Args...) noexcept(Noexcept)>                                    \
    {                                                                                         \
        static constexpr auto is_const = const_;                                              \
        static constexpr auto is_volatile = volatile_;                                        \
        static constexpr auto ref_type = ref_qualifier::ref_;                                 \
                                                                                              \
        using class_t = ClassT;                                                               \
        using qualified_class_t = apply_qualifiers<class_t, is_const, is_volatile, ref_type>; \
    };

#define STDSHARP_MEMBER_FUNCTION_TRAITS_CONST_PACK(is_volatile, ref_type, qualifiers) \
    STDSHARP_MEMBER_FUNCTION_TRAITS(true, is_volatile, ref_type, const qualifiers)    \
    STDSHARP_MEMBER_FUNCTION_TRAITS(false, is_volatile, ref_type, qualifiers)

#define STDSHARP_MEMBER_FUNCTION_TRAITS_VOLATILE_PACK(ref_type, qualifiers)         \
    STDSHARP_MEMBER_FUNCTION_TRAITS_CONST_PACK(true, ref_type, volatile qualifiers) \
    STDSHARP_MEMBER_FUNCTION_TRAITS_CONST_PACK(false, ref_type, qualifiers)

    STDSHARP_MEMBER_FUNCTION_TRAITS_VOLATILE_PACK(none, )
    STDSHARP_MEMBER_FUNCTION_TRAITS_VOLATILE_PACK(lvalue, &)
    STDSHARP_MEMBER_FUNCTION_TRAITS_VOLATILE_PACK(rvalue, &&)

#undef STDSHARP_MEMBER_FUNCTION_TRAITS_REF_PACK
#undef STDSHARP_MEMBER_FUNCTION_TRAITS_VOLATILE_PACK
#undef STDSHARP_MEMBER_FUNCTION_TRAITS_CONST_PACK

    template<typename T, typename ClassT>
    concept member_of = std::same_as<typename member_traits<T>::class_t, ClassT>;

    template<auto Ptr>
    using member_t = member_pointer_traits<Ptr>::type;

    template<bool Noexcept, typename Ret, typename T, typename... Args>
    using mem_func_pointer = details::mem_func_pointer<Noexcept, Ret, T, Args...>::type;

    template<typename T>
    inline constexpr auto type_info = std::ref(typeid(T));
}
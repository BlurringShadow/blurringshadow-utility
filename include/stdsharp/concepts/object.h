#pragma once

#include "../functional/invoke_r.h"
#include "type.h"

namespace stdsharp
{
    template<typename T>
    concept trivial = std::is_trivial_v<T>;

    template<typename T>
    concept trivial_copyable = std::is_trivially_copyable_v<T>;

    template<typename T>
    concept trivial_constructible = std::is_trivially_constructible_v<T>;

    template<typename T>
    concept trivial_copy_constructible = std::is_trivially_copy_constructible_v<T>;

    template<typename T>
    concept trivial_copy_assignable = std::is_trivially_copy_assignable_v<T>;

    template<typename T>
    concept trivial_move_constructible = std::is_trivially_move_constructible_v<T>;

    template<typename T>
    concept trivial_move_assignable = std::is_trivially_move_assignable_v<T>;

    template<typename T>
    concept move_assignable = std::assignable_from<T&, T>;

    template<typename T>
    concept copy_assignable = std::assignable_from<T&, const T&>;

    template<typename T, typename U>
    concept nothrow_convertible_to = std::is_nothrow_convertible_v<T, U>;

    template<typename T>
    concept inheritable = class_<T> && !final<T>;

    template<typename T, typename U>
    concept assignable_to = std::assignable_from<U, T>;

    template<typename From, typename To>
    concept explicitly_convertible = requires(From&& v) { static_cast<To>(cpp_forward(v)); };

    template<typename From, typename To>
    concept nothrow_explicitly_convertible =
        requires(From&& v) { requires noexcept(static_cast<To>(cpp_forward(v))); };

    template<typename To, typename From>
    concept explicitly_convertible_from = explicitly_convertible<From, To>;

    template<typename To, typename From>
    concept nothrow_explicitly_convertible_from = nothrow_explicitly_convertible<From, To>;

    template<typename T, typename U>
    concept convertible_from = std::convertible_to<U, T>;

    template<typename T, typename U>
    concept nothrow_convertible_from = std::is_nothrow_convertible_v<U, T>;

    template<typename T, typename U>
    concept inter_convertible = std::convertible_to<T, U> && convertible_from<T, U>;

    template<typename T, typename U>
    concept nothrow_inter_convertible = nothrow_convertible_to<T, U> &&
        nothrow_convertible_from<T, U>;

    template<typename T, typename U>
    concept explicitly_inter_convertible = explicitly_convertible<T, U> &&
        explicitly_convertible_from<T, U>;

    template<typename T, typename U>
    concept nothrow_explicitly_inter_convertible = nothrow_explicitly_convertible<T, U> &&
        nothrow_explicitly_convertible_from<T, U>;

    template<typename B>
    concept boolean_testable = std::convertible_to<B, bool> && requires(B&& b) {
        { !cpp_forward(b) } -> std::convertible_to<bool>;
    };

    template<typename B>
    concept nothrow_boolean_testable = nothrow_convertible_to<B, bool> && requires(B&& b) {
        { !cpp_forward(b) } noexcept -> nothrow_convertible_to<bool>;
    };

    template<typename T, typename U>
    concept weakly_equality_comparable_with =
        requires(const std::remove_reference_t<T>& t, const std::remove_reference_t<U>& u) {
            { t == u } -> boolean_testable;
            { t != u } -> boolean_testable;
            { u == t } -> boolean_testable;
            { u != t } -> boolean_testable;
        };

    template<typename T, typename U>
    concept nothrow_weakly_equality_comparable_with =
        requires(const std::remove_reference_t<T>& t, const std::remove_reference_t<U>& u) {
            { t == u } noexcept -> nothrow_boolean_testable;
            { t != u } noexcept -> nothrow_boolean_testable;
            { u == t } noexcept -> nothrow_boolean_testable;
            { u != t } noexcept -> nothrow_boolean_testable;
        };

    template<typename T>
    concept weakly_equality_comparable = weakly_equality_comparable_with<T, T>;

    template<typename T>
    concept nothrow_weakly_equality_comparable = nothrow_weakly_equality_comparable_with<T, T>;

    template<typename T, typename U>
    concept partially_ordered_with =
        requires(const std::remove_reference_t<T>& t, const std::remove_reference_t<U>& u) {
            { t < u } -> boolean_testable;
            { t > u } -> boolean_testable;
            { t <= u } -> boolean_testable;
            { t <= u } -> boolean_testable;
            { u < t } -> boolean_testable;
            { u > t } -> boolean_testable;
            { u <= t } -> boolean_testable;
            { u <= t } -> boolean_testable;
        };

    template<typename T>
    concept partially_ordered = partially_ordered_with<T, T>;

    template<typename T, typename... Args>
    concept list_initializable_from = requires { T{std::declval<Args>()...}; };

    template<typename T, typename... Args>
    concept nothrow_list_initializable_from =
        requires { requires noexcept(T{std::declval<Args>()...}); };

    template<typename T, typename... Args>
    concept implicitly_constructible_from = std::constructible_from<T, Args...> &&
        requires { std::declval<void(const T&)>()({std::declval<Args>()...}); };

    template<typename T>
    concept implicitly_move_constructible = std::move_constructible<T> &&
        implicitly_constructible_from<T, T>;

    template<typename T>
    concept implicitly_copy_constructible = std::copy_constructible<T> && //
        implicitly_move_constructible<T> && //
        implicitly_constructible_from<T, T&> && //
        implicitly_constructible_from<T, const T> && //
        implicitly_constructible_from<T, const T&>;

    template<typename T, typename... Args>
    concept nothrow_constructible_from = std::is_nothrow_constructible_v<T, Args...>;

    template<typename T>
    concept nothrow_default_initializable = std::default_initializable<T> &&
        std::is_nothrow_default_constructible_v<T>;

    template<typename T>
    concept nothrow_move_constructible = std::is_nothrow_move_constructible_v<T>;

    template<typename T>
    concept nothrow_copy_constructible = std::is_nothrow_copy_constructible_v<T>;

    template<typename T, typename U>
    concept nothrow_assignable_from = std::assignable_from<T, U> &&
        std::is_nothrow_assignable_v<T, U>;

    template<typename T>
    concept nothrow_copy_assignable = copy_assignable<T> && std::is_nothrow_copy_assignable_v<T>;

    template<typename T>
    concept nothrow_move_assignable = move_assignable<T> && std::is_nothrow_move_assignable_v<T>;

    template<typename T, typename U>
    concept nothrow_swappable_with =
        requires(T& t, U& u) { requires noexcept(std::ranges::swap(t, u)); };

    template<typename T>
    concept nothrow_swappable = nothrow_swappable_with<T, T>;

    template<typename T>
    concept nothrow_movable = std::movable<T> && //
        nothrow_move_constructible<T> &&
        nothrow_assignable_from<std::add_lvalue_reference_t<T>, T> && //
        nothrow_swappable<T>;

    template<typename T>
    concept nothrow_copyable = requires(
        std::add_lvalue_reference_t<T> ref,
        std::add_const_t<T> const_,
        std::add_lvalue_reference_t<decltype(const_)> const_ref
    ) {
        requires nothrow_movable<T>;
        requires nothrow_copy_constructible<T>;
        requires nothrow_assignable_from<decltype(ref), decltype(ref)>;
        requires nothrow_assignable_from<decltype(ref), decltype(const_)>;
        requires nothrow_assignable_from<decltype(ref), decltype(const_ref)>;
    };

    template<typename Func, typename... Args>
    concept nothrow_invocable = std::is_nothrow_invocable_v<Func, Args...>;

    template<typename Func, typename ReturnT, typename... Args>
    concept regular_invocable_r = std::regular_invocable<Func, Args...> &&
        invocable_r<Func, ReturnT, Args...>;

    template<typename Func, typename ReturnT, typename... Args>
    concept nothrow_regular_invocable_r = regular_invocable_r<Func, ReturnT, Args...> &&
        nothrow_invocable_r<Func, ReturnT, Args...>;

    template<typename Func, typename... Args>
    concept nothrow_predicate = nothrow_invocable_r<Func, bool, Args...>;

    template<typename T>
    concept nullable_pointer = std::default_initializable<T> && //
        std::copy_constructible<T> && //
        copy_assignable<T> && //
        std::equality_comparable<T> && //
        weakly_equality_comparable_with<T, std::nullptr_t>;

    template<typename T>
    concept referenceable = requires {
        requires not_same_as<void, T>;
        typename std::type_identity<T&>;
    };

    template<typename T>
    concept dereferenceable = requires(T&& t) {
        { *cpp_forward(t) } -> referenceable;
    };

    template<typename T>
    concept nothrow_dereferenceable = dereferenceable<T> && requires(T& t) {
        { *t } noexcept;
    };

    template<typename T>
    concept arithmetic_like = std::three_way_comparable<T> && requires(T t1, T t2) {
        { t1 + t2 } -> std::same_as<T>;
        { t1 - t2 } -> std::same_as<T>;
        { t1* t2 } -> std::same_as<T>;
        { t1 / t2 } -> std::same_as<T>;
        { t1 % t2 } -> std::same_as<T>;
        { t1& t2 } -> std::same_as<T>;
        { t1 | t2 } -> std::same_as<T>;
        { t1 ^ t2 } -> std::same_as<T>;
        { t1 << t2 } -> std::same_as<T>;
        { t1 >> t2 } -> std::same_as<T>;

        { t1 += t2 } -> std::same_as<T&>;
        { t1 -= t2 } -> std::same_as<T&>;
        { t1 *= t2 } -> std::same_as<T&>;
        { t1 /= t2 } -> std::same_as<T&>;
        { t1 %= t2 } -> std::same_as<T&>;
        { t1 &= t2 } -> std::same_as<T&>;
        { t1 |= t2 } -> std::same_as<T&>;
        { t1 ^= t2 } -> std::same_as<T&>;
        { t1 <<= t2 } -> std::same_as<T&>;
        { t1 >>= t2 } -> std::same_as<T&>;
    };
}

#pragma once

#include "../namespace_alias.h"

#include <chrono>
#include <concepts>
#include <mutex> // IWYU pragma: export

namespace stdsharp
{
    template<typename T>
    concept basic_lockable = requires(T t) {
        t.lock();
        t.unlock();
    };

    template<typename T>
    concept lockable = requires(T t) {
        requires basic_lockable<T>;
        { t.try_lock() } -> std::same_as<bool>;
    };

    template<typename T>
    concept timed_lockable = requires(T t) {
        requires lockable<T>;
        { t.try_lock_for(std::chrono::seconds{1}) } -> std::same_as<bool>;
        { t.try_lock_until(std::chrono::system_clock::now()) } -> std::same_as<bool>;
    };

    template<typename T>
    concept shared_lockable = requires(T t) {
        t.lock_shared();
        t.unlock_shared();
        { t.try_lock_shared() } -> std::same_as<bool>;
    };

    template<typename T>
    concept shared_timed_lockable = requires(T t) {
        requires shared_lockable<T>;
        { t.try_lock_shared_for(std::chrono::seconds{1}) } -> std::same_as<bool>;
        { t.try_lock_shared_until(std::chrono::system_clock::now()) } -> std::same_as<bool>;
    };

    template<typename T>
    concept mutex = requires(T t) {
        requires lockable<T>;
        requires !std::movable<T>;
        requires std::default_initializable<T>;
        { t.lock() } -> std::same_as<void>;
        { t.unlock() } -> std::same_as<void>;
    };

    template<typename T>
    concept timed_mutex = timed_lockable<T> && mutex<T>;

    template<typename T>
    concept shared_mutex = requires(T t) {
        requires mutex<T>;
        requires shared_lockable<T>;
        { t.lock_shared() } -> std::same_as<void>;
        { t.unlock_shared() } -> std::same_as<void>;
    };

    template<typename T>
    concept shared_timed_mutex = timed_mutex<T> && shared_mutex<T> && shared_timed_lockable<T>;
}

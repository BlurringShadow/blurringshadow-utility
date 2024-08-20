#pragma once

#include "concepts/object.h"
#include "utility/forward_cast.h"

#include <memory>

namespace stdsharp
{
    template<std::invocable Fn>
    class lazy // NOLINTBEGIN(*-noexcept-*)
    {
    public:
        using value_type = std::invoke_result_t<Fn>;

    private:
        bool has_value_ = false;

        union
        {
            Fn fn_{};
            value_type value_;
        };

        static constexpr void generate_value(auto&& this_) noexcept(
            nothrow_invocable<Fn> &&
            nothrow_constructible_from<value_type, std::invoke_result_t<Fn>> //
        )
        {
            if(this_.has_value()) return;

            auto&& v = std::invoke(cpp_move(this_.fn_));
            std::ranges::destroy_at(&this_.fn_);
            std::ranges::construct_at(&this_.value_, cpp_move(v));
            this_.has_value_ = true;
        }

    public:
        lazy() = default;

        template<typename... Args>
            requires std::constructible_from<Fn, Args...>
        constexpr explicit(sizeof...(Args) == 1) lazy(Args&&... args)
            noexcept(nothrow_constructible_from<Fn, Args...>):
            fn_(cpp_forward(args)...)
        {
        }

    private:
        constexpr void ctor(auto&& other)
        {
            if(has_value_) std::ranges::construct_at(&value_, cpp_forward(other).value_);
            else std::ranges::construct_at(&fn_, cpp_forward(other).fn_);
        }

        constexpr void assign(auto&& other)
        {
            if(has_value_ == other.has_value_)
            {
                if(has_value_) value_ = cpp_forward(other).value_;
                else fn_ = cpp_forward(other).fn_;

                return;
            }

            if(has_value_)
            {
                std::ranges::destroy_at(&value_);
                std::ranges::construct_at(&fn_, cpp_forward(other).fn_);
            }
            else
            {
                std::ranges::destroy_at(&fn_);
                std::ranges::construct_at(&value_, cpp_forward(other).value_);
            }

            has_value_ = !has_value_;
        }

        static constexpr auto self_cast = fwd_cast<lazy>;

    public:
        constexpr lazy(const lazy& other)
            noexcept(nothrow_copy_constructible<Fn> && nothrow_copy_constructible<value_type>)
            requires std::copy_constructible<Fn> && std::copy_constructible<value_type>
            : has_value_(other.has_value_)
        {
            ctor(other);
        }

        constexpr lazy(lazy&& other)
            noexcept(nothrow_move_constructible<Fn> && nothrow_move_constructible<value_type>)
            requires std::move_constructible<Fn> && std::move_constructible<value_type>
            : has_value_(other.has_value_)
        {
            ctor(cpp_move(other));
        }

        constexpr lazy& operator=(const lazy& other)
            noexcept(nothrow_copy_assignable<Fn> && nothrow_copy_assignable<value_type>)
            requires copy_assignable<Fn> && copy_assignable<value_type>
        {
            assign(other);
            return *this;
        }

        constexpr lazy& operator=(lazy&& other)
            noexcept(nothrow_move_assignable<Fn> && nothrow_move_assignable<value_type>)
            requires move_assignable<Fn> && move_assignable<value_type>
        {
            assign(cpp_move(other));
            return *this;
        }

        constexpr ~lazy()
        {
            if(has_value_) std::ranges::destroy_at(&value_);
            else std::ranges::destroy_at(&fn_);
        }

        constexpr void swap(lazy& other) noexcept(
            nothrow_swappable<Fn> && //
            nothrow_swappable<value_type> && //
            nothrow_move_constructible<Fn> && //
            nothrow_move_constructible<value_type>
        )
            requires requires {
                requires std::swappable<Fn>;
                requires std::swappable<value_type>;
                requires std::move_constructible<Fn>;
                requires std::move_constructible<value_type>;
            }
        {
            if(has_value_ == other.has_value_)
            {
                if(has_value_) std::ranges::swap(value_, other.value_);
                else std::ranges::swap(fn_, other.fn_);
                return;
            }

            if(has_value_)
            {
                auto temp = cpp_move(value_);
                std::ranges::destroy_at(&value_);
                std::ranges::construct_at(&fn_, cpp_move(other.fn_));
                std::ranges::destroy_at(&other.fn_);
                std::ranges::construct_at(&other.value_, cpp_move(temp));
            }
            else
            {
                auto temp = cpp_move(fn_);
                std::ranges::destroy_at(&fn_);
                std::ranges::construct_at(&value_, cpp_move(other.value_));
                std::ranges::destroy_at(&other.value_);
                std::ranges::construct_at(&other.fn_, cpp_move(temp));
            }

            has_value_ = !has_value_;
            other.has_value_ = !has_value_;
        }

        [[nodiscard]] constexpr bool has_value() const noexcept { return has_value_; }

        constexpr decltype(auto) get(this auto&& self)
            noexcept(noexcept(generate_value(self_cast(cpp_forward(self)))))
            requires requires { generate_value(self_cast(cpp_forward(self))); }
        {
            auto&& this_ = self_cast(cpp_forward(self));
            generate_value(cpp_forward(this_));
            return cpp_forward(this_).value_;
        }

        constexpr decltype(auto) cget(this const auto&& self) noexcept
        {
            return self_cast(cpp_forward(self)).value_;
        }

        constexpr decltype(auto) cget(this const auto& self) noexcept
        {
            return self_cast(self).value_;
        }
    }; // NOLINTEND(*-noexcept-*)

    template<typename Fn>
    lazy(Fn&&) -> lazy<std::decay_t<Fn>>;
}

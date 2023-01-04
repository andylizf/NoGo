#pragma once

#include <cstdio>
#include <iostream>

using namespace std;

#define CSI "\x1b["

constexpr int rank_n = 9;

struct Pair {
    int x, y;
    constexpr Pair(int x, int y)
        : x(x)
        , y(y)
    {
    }
    constexpr Pair(int x)
        : Pair(x, x)
    {
    }

    // explicit operator bool() { return x != -1 && y != -1; }
    constexpr Pair& operator+=(Pair p)
    {
        x += p.x, y += p.y;
        return *this;
    }
    constexpr Pair operator+(Pair p) const
    {
        Pair res = *this;
        return res += p;
    }
    constexpr Pair& operator-=(Pair p)
    {
        x -= p.x, y -= p.y;
        return *this;
    }
    constexpr Pair operator-(Pair p) const
    {
        Pair res = *this;
        return res -= p;
    }
    constexpr Pair& operator*=(Pair p)
    {
        x *= p.x, y *= p.y;
        return *this;
    }
    constexpr Pair operator*(Pair p) const
    {
        Pair res = *this;
        return res *= p;
    }
    constexpr Pair operator/=(Pair p)
    {
        x /= p.x, y /= p.y;
        return *this;
    }
    constexpr Pair operator/(Pair p) const
    {
        Pair res = *this;
        return res /= p;
    }
    constexpr bool operator==(const Pair& p) const = default;

    static void go(Pair p)
    {
        printf(CSI "%d;%dH", p.x, p.y);
    }
    template <typename... Ts>
    static void print(Pair p, Ts... args)
    {
        go(p);
        (cout << ... << args);
    }
    template <typename... Ts>
    static void println(Pair& p, Ts... args)
    {
        go(p);
        (cout << ... << args);
        p.x++;
    }
};

struct Pos : public Pair {
    using Pair::Pair;
    static constexpr int uninited = -2;
    constexpr Pos()
        : Pair(uninited)
    {
    }
    constexpr Pos(Pair p)
        : Pair(p)
    {
    }

    constexpr char get_digit() { return x != uninited ? rank_n - x + '0' : ' '; }
    constexpr Pos& set_digit(char i)
    {
        x = (i != ' ' ? rank_n - (i - '0') : uninited);
        return *this;
    }
    constexpr char get_alpha() { return y != uninited ? y + 'A' : ' '; }
    constexpr Pos& set_alpha(char i)
    {
        y = (i != ' ' ? i - 'A' : uninited);
        return *this;
    }

    friend ostream& operator<<(ostream& out, Pos p)
    {
        return out << string(1, p.get_digit())
                   << string(1, p.get_alpha());
    }
    friend istream& operator>>(istream& in, Pos& p)
    {
        char digit, alpha;
        in >> digit >> alpha;
        p.set_alpha(alpha).set_digit(digit);
        return in;
    }
};

#include <chrono>
#include <future>
#include <optional>
#include <type_traits>

/*
// A shortcut to the result type of F(Args...).
template <typename F, typename... Args>
using result_t = std::invoke_result_t<std::decay_t<F>,(std::decay_t<Args>...)>;
*/

// A shortcut to the type of a duration D.
template <typename D>
using duration_t = std::chrono::duration<
    typename D::rep, typename D::period>;

// Run a function asynchronously if timeout is non-zero.
//
// The return value is an optional<result_t>.
// The optional is "empty" if the async execution timed out.
template <typename TO, typename F, typename... Args>
inline constexpr auto with_timeout(const TO& timeout, F&& f, Args&&... args)
{
    if (timeout == duration_t<TO>::zero())
        return std::optional { f(args...) };

    auto future = std::async(std::launch::async,
        std::forward<F>(f), std::forward<Args...>(args...));
    auto status = future.wait_for(timeout);

    return status == std::future_status::ready
        ? std::optional { future.get() }
        : std::nullopt;
}

template <typename T>
inline std::string to_string(const T& value)
{
    std::ostringstream ss;
    ss << value;
    return ss.str();
}

#define strcat_(x, y) x##y
#define strcat(x, y) strcat_(x, y)
#define PRINT_VALUE(x)                        \
    template <int>                            \
    struct strcat(strcat(value_of_, x), _is); \
    static_assert(strcat(strcat(value_of_, x), _is) < x > ::x, "");
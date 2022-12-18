#pragma once

#include <cstdio>
#include <iostream>

using namespace std;

#define CSI "\x1b["

constexpr int rank_n = 9;

struct Pair {
    int x, y;
    Pair(int x, int y)
        : x(x)
        , y(y)
    {
    }
    Pair(int x)
        : Pair(x, x)
    {
    }

    // explicit operator bool() { return x != -1 && y != -1; }
    Pair& operator+=(Pair p)
    {
        x += p.x, y += p.y;
        return *this;
    }
    Pair operator+(Pair p) const
    {
        Pair res = *this;
        return res += p;
    }
    Pair& operator-=(Pair p)
    {
        x -= p.x, y -= p.y;
        return *this;
    }
    Pair operator-(Pair p) const
    {
        Pair res = *this;
        return res -= p;
    }
    Pair operator-() const { return Pair { 0, 0 } - *this; }
    Pair& operator*=(Pair p)
    {
        x *= p.x, y *= p.y;
        return *this;
    }
    Pair operator*(Pair p) const
    {
        Pair res = *this;
        return res *= p;
    }
    Pair operator/=(Pair p)
    {
        x /= p.x, y /= p.y;
        return *this;
    }
    Pair operator/(Pair p) const
    {
        Pair res = *this;
        return res /= p;
    }
    auto operator<=>(const Pair& p) const = default;

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
    static inline int uninited = -2;
    Pos()
        : Pair(uninited)
    {
    }
    Pos(Pair p)
        : Pair(p)
    {
    }

    char get_digit() { return x != uninited ? rank_n - x + '0' : ' '; }
    Pos& set_digit(char i)
    {
        x = (i != ' ' ? rank_n - (i - '0') : uninited);
        return *this;
    }
    char get_alpha() { return y != uninited ? y + 'A' : ' '; }
    Pos& set_alpha(char i)
    {
        y = (i != ' ' ? i - 'A' : uninited);
        return *this;
    }

    friend ostream& operator<<(ostream& out, Pos p) { return out << to_string(p); }
    friend istream& operator>>(istream& in, Pos& p)
    {
        char digit, alpha;
        in >> digit >> alpha;
        p.set_alpha(alpha).set_digit(digit);
        return in;
    }
    friend string to_string(Pos p)
    {
        return string(1, p.get_digit())
            + string(1, p.get_alpha());
    }
};
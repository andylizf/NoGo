#pragma once

#include <array>
#include <cassert>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <ranges>
#include <vector>

using namespace std;
namespace fs = std::filesystem;
namespace ranges = std::ranges;

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

class BoardType {
    array<int, rank_n * rank_n> arr;

public:
    constexpr int& operator[](Pos p) { return arr[p.x * rank_n + p.y]; }
    constexpr int operator[](Pos p) const { return arr[p.x * rank_n + p.y]; }

    constexpr bool in_border(Pos p) const { return p.x >= 0 && p.y >= 0 && p.x < rank_n && p.y < rank_n; }

    static constexpr auto index()
    {
        array<Pos, rank_n * rank_n> res;
        for (auto i : views::iota(0, rank_n))
            for (auto j : views::iota(0, rank_n))
                res[i * rank_n + j] = { i, j };
        return res;
    }

    static constexpr array delta { Pair { -1, 0 }, Pair { 1, 0 }, Pair { 0, -1 }, Pair { 0, 1 } };

    constexpr bool liberties(this BoardType const& self, Pos p)
    {
        BoardType visit {};
        auto _liberties = [self, &visit](this auto&& fself, Pos p) -> bool {
            visit[p] = true;
            return ranges::any_of(delta, [self, p, fself, &visit](auto d) {
                Pos n = p + d;
                return self.in_border(n)
                    && (!self[n] || self[n] == self[p] && !visit[n] && fself(n));
            });
        };
        return _liberties(p);
    }

    // judge whether stones around `p` is captured by `p`
    // or `p` is captured by stones around `p`
    constexpr bool is_capturing(this BoardType const& self, Pos p)
    {
        assert(self[p]);

        return !self.liberties(p)
            || ranges::any_of(delta, [p, self](auto d) {
                   Pos n = p + d;
                   return self.in_border(n) && self[n] == -self[p]
                       && !self.liberties(n);
               });
    }
};

struct RoleType {
    enum Value {
        WHITE = -1,
        BLACK = 1
    };
    constexpr RoleType(Value value)
        : value(value)
    {
    }
    constexpr operator int() const
    {
        return value;
    }
    constexpr explicit operator bool() const
    {
        return value == BLACK;
    }
    constexpr void reverse(this RoleType& self)
    {
        self = self ? WHITE : BLACK;
    }
    friend ostream& operator<<(ostream& out, RoleType role)
    {
        return out << (role ? "BLACK" : "WHITE");
    }

private:
    Value value;
};

class State {
public:
    BoardType board {};
    vector<Pos> moves {};
    RoleType role;

    constexpr State(RoleType role = RoleType::BLACK)
        : role(role)
    {
    }

    constexpr State next_state(this State const& self, Pos p)
    {
        assert(!board[p]);

        auto state { self };
        state.board[p] = state.role;
        state.moves.push_back(p);
        state.role.reverse();
        return state;
    }

    constexpr Pos revoke()
    {
        if (!moves.size())
            return {};

        auto p { moves.back() };
        moves.pop_back();
        board[p] = 0;
        role.reverse();
        return p;
    }

    constexpr int is_over() const
    {
        if (moves.size() && board.is_capturing(moves.back())) // win
            return role;
        if (!available_actions().size()) // lose
            return -role;
        return 0;
    }
    constexpr vector<Pos> available_actions() const
    {
        auto temp_board { board };
        return BoardType::index() | views::filter([&temp_board, this](auto pos) {
            if (temp_board[pos])
                return false;
            temp_board[pos] = this->role;
            bool res { !temp_board.is_capturing(pos) };
            temp_board[pos] = 0;
            return res;
        }) | ranges::to<std::vector>();
    }
};
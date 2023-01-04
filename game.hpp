#pragma once

#include <array>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <ranges>
#include <vector>

#include "pair.hpp"

using namespace std;
namespace fs = std::filesystem;
namespace ranges = std::ranges;

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

template <typename PlayerType>
class Contest {
public:
    class StonePositionOccupiedException : public std::logic_error {
        using logic_error::logic_error;
    };
    class TimeLimitExceededException : public std::runtime_error {
        using runtime_error::runtime_error;
    };

    State current {};
    PlayerType player1, player2;
    int winner { 0 };
    constexpr Contest(const PlayerType& player1, const PlayerType& player2)
        : player1(player1)
        , player2(player2)
    {
    }
    /*
    example.nogo only saves the situation
    1A
    2D
    6E
    3A
    */

    constexpr int round() const
    {
        return (int)current.moves.size();
    }

    void save(fs::path path)
    {
        ofstream writeFile(path, ios::out);
        for (auto p : current.moves)
            writeFile << p << '\n';
        writeFile.close();
    }
    void load(fs::path path)
    {
        ifstream readFile(path, ios::in);
        Pos p {};
        while (readFile >> p)
            current.next_state(p);
        readFile.close();
    }

    constexpr bool play()
    {
        auto player { current.role ? player1 : player2 };
        auto p { with_timeout(1000ms, player, current) };
        if (!p) {
            winner = -current.role;
            throw TimeLimitExceededException { to_string(current.role) + " exceeds the time limit." };
        }
        if (current.board[*p]) {
            winner = -current.role;
            throw StonePositionOccupiedException { to_string(current.role) + " choose a occupied position." };
        }
        current = current.next_state(*p);
        winner = current.is_over();
        return !winner;
    }
};
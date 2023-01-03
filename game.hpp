#pragma once

#include <array>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <vector>

#include "pair.hpp"

using namespace std;
namespace fs = std::filesystem;

struct BoardType {
    array<int, rank_n * rank_n> arr;
    int& operator[](Pos p) { return arr[p.x * rank_n + p.y]; }
    int operator[](Pos p) const { return arr[p.x * rank_n + p.y]; }

    bool in_border(Pos p) const { return p.x >= 0 && p.y >= 0 && p.x < rank_n && p.y < rank_n; }

    static auto index()
    {
        array<Pos, rank_n * rank_n> res;
        for (int i = 0; i < rank_n; i++)
            for (int j = 0; j < rank_n; j++)
                res[i * rank_n + j] = { i, j };
        return res;
    }

    static inline Pair delta[] = { Pair { -1, 0 }, Pair { 1, 0 }, Pair { 0, -1 }, Pair { 0, 1 } };
    bool _liberties(Pos p, BoardType& visit) const
    {
        visit[p] = true;
        for (auto d : delta) {
            Pos n = p + d;
            if (!in_border(n))
                continue;
            if (!(*this)[n])
                return true;
            if ((*this)[n] == (*this)[p] && !visit[n]
                && _liberties(n, visit))
                return true;
        }
        return false;
    }

    bool liberties(Pos p) const
    {
        BoardType visit {};
        return _liberties(p, visit);
    }

    // judge whether stones around `p` is captured by `p`
    // or `p` is captured by stones around `p`
    bool is_capturing(Pos p) const
    {
        assert((*this)[p]);
        if (!liberties(p))
            return true;
        for (auto d : delta) {
            Pos n = p + d;
            if (in_border(n) && (*this)[n] == -(*this)[p]
                && !liberties(n))
                return true;
        }
        return false;
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
    constexpr void reverse()
    {
        *this = *this ? WHITE : BLACK;
    }

private:
    Value value;
};

class State {
public:
    BoardType board {};
    vector<Pos> moves {};
    RoleType role { RoleType::BLACK };

    State next_state(Pos p) const
    {
        if (board[p])
            throw string("invalid stone position for role") + (role ? "black" : "white");
        auto state = *this;
        state.board[p] = role;
        state.moves.push_back(p);
        state.role.reverse();
        return state;
    }

    Pos revoke()
    {
        if (!moves.size())
            return {};
        auto p = moves.back();
        moves.pop_back();
        board[p] = 0;
        role.reverse();
        return p;
    }

    int is_over() const
    {
        if (moves.size() && board.is_capturing(moves.back())) // win
            return role;
        if (!available_actions().size()) // lose
            return -role;
        return 0;
    }
    vector<Pos> available_actions() const
    {
        vector<Pos> actions {};
        auto temp_board { board };
        for (auto pos : board.index()) {
            if (board[pos])
                continue;
            temp_board[pos] = role;
            if (!temp_board.is_capturing(pos))
                actions.push_back(pos);
            temp_board[pos] = 0;
        }
        return actions;
    }
};

class Contest {
public:
    State current {};
    using PlayerType = function<Pos(State)>;
    PlayerType player1, player2;
    int winner { 0 };
    Contest(PlayerType&& player1, PlayerType&& player2)
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

    int round() const
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
        while (readFile.good() && readFile >> p) {
            current.next_state(p);
        }
        readFile.close();
    }

    bool play()
    {
        auto&& player = current.role ? player1 : player2;
        auto res = with_timeout(1000ms, player, current);
        if (!res) {
            winner = -current.role;
            cout << "timeout" << endl;
            return false;
        }
        current = current.next_state(*res);
        winner = current.is_over();
        return !winner;
    }
};
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
    BoardType() { arr.fill(0); }
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
            if ((*this)[n] == (*this)[p] && !visit[n])
                return _liberties(n, visit);
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
            return false;
        for (auto d : delta) {
            Pos n = p + d;
            if (in_border(n) && (*this)[n] == -(*this)[p]
                && !liberties(n))
                return true;
        }
        return false;
    }
};

class Contest {
public:
    using PlayerType = function<Pos(BoardType, bool)>;
    BoardType board;
    vector<Pos> moves;
    PlayerType player1, player2;
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

    Pos revoke()
    {
        if (!moves.size())
            return {};
        auto p = moves.back();
        moves.pop_back();
        board[p] = 0;
        return p;
    }
    int round() const
    {
        return (int)moves.size();
    }

    void save(fs::path path)
    {
        ofstream writeFile(path, ios::out);
        for (auto p : moves)
            writeFile << p << '\n';
        writeFile.close();
    }
    void load(fs::path path)
    {
        ifstream readFile(path, ios::in);
        while (readFile.good()) {
            Pos p;
            readFile >> p;
            moves.push_back(p);
        }
        readFile.close();
    }

    bool is_valid(Pos p)
    {
        if (board[p])
            return false;
        // TODO
        return true;
    }

    bool play()
    {
        bool isblack = moves.size() % 2 == 0;
        auto p = (isblack ? player1 : player2)(board, isblack);
        if (is_valid(p))
            board[p] = isblack ? 1 : -1, moves.push_back(p);
        else
            throw string("invalid stone position for player") + (isblack ? "black" : "white");

        if (board.is_capturing(p))
            return false;
        return moves.size() != rank_n * rank_n;
    }
};

inline Pos bot_player(const BoardType& _board, bool isblack)
{
    vector<Pos> solutions;
    BoardType board = _board;
    for (auto pos : board.index())
        if (!board[pos]) {
            board[pos] = isblack ? 1 : -1;
            if (!board.is_capturing(pos))
                solutions.push_back(pos);
            board[pos] = 0;
        }
    return solutions[rand() % solutions.size()];
};

#pragma once

#include <chrono>
#include <future>
#include <optional>
#include <type_traits>

#include "Rule/Rule.hpp"

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
inline auto with_timeout(const TO& timeout, F&& f, Args&&... args)
{
    if (timeout == duration_t<TO>::zero())
        return std::optional { f(args...) };

    // std::printf("launching...\n");
    auto future = std::async(std::launch::async,
        std::forward<F>(f), std::forward<Args...>(args...));
    auto status = future.wait_for(timeout);

    return status == std::future_status::ready
        ? std::optional { future.get() }
        : std::nullopt;
}


class Contest {
public:
    class StonePositionOccupiedException : public std::logic_error {
        using logic_error::logic_error;
    };
    class TimeLimitExceededException : public std::runtime_error {
        using runtime_error::runtime_error;
    };

    State current {};
    using PlayerType = function<Pos(State)>;
    PlayerType player1, player2;
    int winner { 0 };
    Contest(const PlayerType& player1, const PlayerType& player2)
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
        while (readFile >> p)
            current.next_state(p);
        readFile.close();
    }

    bool play()
    {
        auto&& player { current.role ? player1 : player2 };
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
#include <format>
#include <iostream>

#include "Bot/Bot.hpp"
#include "Contest.hpp"

using namespace std;

template <typename T>
double battle(int cnt, T&& player1, T&& player2)
{
    auto win_cnt = ranges::count_if(views::iota(0, cnt), [player1, player2](auto _) {
        Contest contest(player1, player2);
        while (contest.play())
            ;
        return contest.winner == 1;
    });
    return (double)win_cnt / cnt;
}

int main()
{
    double eps = 0.1;
    for (double C = 1; C <= 2; C += eps) {
        cout << format("Case C={:.1f}:", C);
        double rate = battle(10, mcts_bot_player_generator(C), mcts_bot_player_generator(C + eps));
        cout << format("{:.1f}", rate) << endl;
    }
}
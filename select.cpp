#include <chrono>
#include <format>
#include <iostream>

#include "bot.hpp"

using namespace std;
using namespace std::chrono;

#ifdef SELECT
int main()
{
    double eps = 0.1;
    for (double C = 0.1; C <= 2; C += eps) {
        cout << format("Case C={}:", C) << endl;
        auto mcts_bot_player = mcts_bot_player_generator(C);
        int win_cnt = 0;
        int cnt = 20;
        for (int i = 0; i < cnt; i++) {
            Contest contest(random_bot_player, mcts_bot_player);

            while (contest.play())
                ;
            // cout << endl;
            if (contest.winner == -1)
                win_cnt++, cout << "WIN" << endl;
            else
                cout << "LOSE" << endl;
        }
        double rate = (double)win_cnt / cnt;
        cout << "Rate: " << rate << endl;
    }
}
#endif
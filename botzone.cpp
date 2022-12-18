#include "game.hpp"

#include "json/json.h"

#ifdef botzone

int main()
{
    srand((unsigned)time(0));
    string str;
    getline(cin, str);
    Json::Value input;
    Json::Reader().parse(str, input);

    auto get = [&input](const char* s, auto i) -> Pos {
        return { input[s][i]["x"].asInt(), input[s][i]["y"].asInt() };
    };

    bool isblack = get("requests", 0U) == Pos { -1, -1 };
    State state { isblack };
    if (!isblack)
        state.put(get("requests", 0U));

    for (auto i = 0; i != input["responses"].size(); i++) {
        state.put(get("responses", i));
        state.put(get("requests", i + 1));
    }

    Pair result = random_bot_player(state);

    Json::Value action;
    action["x"] = result.x, action["y"] = result.y;
    Json::Value ret;
    ret["response"] = action;

    cout << Json::FastWriter().write(ret) << endl;
}
#endif
#include "bot.hpp"

#include "json/json.h"

#ifdef BOTZONE
int main()
{
	string str;
	getline(cin, str);
	Json::Value input;
	Json::Reader().parse(str, input);

	auto get = [&input](const char* s, unsigned i) -> Pos {
		return { input[s][i]["x"].asInt(), input[s][i]["y"].asInt() };
	};

	auto role = get("requests", 0) == -1 ? RoleType::BLACK : RoleType::WHITE;
	State state{ role };
	if (!role)
		state = state.next_state(get("requests", 0));

	for (auto i = 0; i != input["responses"].size(); i++) {
		state = state.next_state(get("responses", i));
		state = state.next_state(get("requests", i + 1));
	}

	Pair result = mcts_bot_player(state);

	Json::Value action;
	action["x"] = result.x, action["y"] = result.y;
	Json::Value ret;
	ret["response"] = action;

	cout << Json::FastWriter().write(ret) << endl;
}
#endif
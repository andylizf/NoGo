#include "game.hpp"
#pragma once

#undef botzone
#ifdef botzone

#include "json/json.h"

constexpr int rank_n = 9;
int main()
{
	srand((unsigned)time(0));
	string str; getline(cin, str);
	Json::Value input; Json::Reader().parse(str, input);

	auto board = vector(rank_n, vector(rank_n, 0));
	for (auto request : input["requests"]) {
		int x = request["x"].asInt(), y = request["y"].asInt();
		if (x != -1 && y != -1) board[x][y] = 1;
	}
	for (auto response : input["responses"]) {
		int x = response["x"].asInt(), y = response["y"].asInt();
		if (x != -1 && y != -1) board[x][y] = -1;
	}
	bool isblack = input["requests"].size() == input["responses"].size() ? 1 : -1;
	Pair result = bot_player(board, isblack);

	Json::Value action; action["x"] = result.x, action["y"] = result.y;
	Json::Value ret; ret["response"] = action;

	cout << Json::FastWriter().write(ret) << endl;
}

#endif
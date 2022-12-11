#pragma once

#include <vector>
#include <functional>
#include <filesystem>

#include "pair.hpp"

using namespace std;
namespace filesystem = std::filesystem;

class Contest
{
public:
	using BoardType = vector <vector<int>>;
	using PlayerType = function <Pair(BoardType, bool)>;
	int rank_n;
	BoardType board;
	PlayerType player1, player2;
	Contest(int rank_n, PlayerType&& player1, PlayerType&& player2) :
		rank_n(rank_n), board(vector(rank_n, vector(rank_n, 0))), player1(player1), player2(player2) {}
	void save(filesystem::path path)
	{

	}
	void load(filesystem::path path)
	{

	}
	bool is_valid(Pair pos)
	{
		if (board[pos.x][pos.y]) return false;
		// TODO
		return true;
	}
	int move_n = 0;
	bool play()
	{
		if (move_n % 2 == 0) {
			auto pos = player1(board, true);
			if (is_valid(pos)) board[pos.x][pos.y] = 1;
			else throw "invalid stone position for player1";
		}
		else {
			auto pos = player2(board, false);
			if (is_valid(pos)) board[pos.x][pos.y] = -1;
			else throw "invalid stone position for player2";
		}
		return ++move_n != rank_n * rank_n;
	}
};

constexpr int cx[] = { -1,0,1,0 };
constexpr int cy[] = { 0,-1,0,1 };
inline bool inBorder(int x, int y) { return x >= 0 && y >= 0 && x < 9 && y < 9; }
inline bool dfs_air_visit[9][9];
inline bool dfs_air(Contest::BoardType board, int fx, int fy) //true: has air
{
	dfs_air_visit[fx][fy] = true;
	bool flag = false;
	for (int dir = 0; dir < 4; dir++)
	{
		int dx = fx + cx[dir], dy = fy + cy[dir];
		if (inBorder(dx, dy))
		{
			if (board[dx][dy] == 0)
				flag = true;
			if (board[dx][dy] == board[fx][fy] && !dfs_air_visit[dx][dy])
				if (dfs_air(board, dx, dy))
					flag = true;
		}
	}
	return flag;
};
//true: available
inline bool judgeAvailable(Contest::BoardType board, int fx, int fy, int col)
{
	if (board[fx][fy]) return false;
	board[fx][fy] = col;
	memset(dfs_air_visit, 0, sizeof(dfs_air_visit));
	if (!dfs_air(board, fx, fy))
	{
		board[fx][fy] = 0;
		return false;
	}
	for (int dir = 0; dir < 4; dir++)
	{
		int dx = fx + cx[dir], dy = fy + cy[dir];
		if (inBorder(dx, dy))
		{
			if (board[dx][dy] && !dfs_air_visit[dx][dy])
				if (!dfs_air(board, dx, dy))
				{
					board[fx][fy] = 0;
					return false;
				}
		}
	}
	board[fx][fy] = 0;
	return true;
};

inline Pair bot_player(const Contest::BoardType& board, bool isblack) {
	vector<int> available_list;
	int rank_n = (int)board.size();
	for (int i = 0; i < rank_n; i++)
		for (int j = 0; j < rank_n; j++)
			if (judgeAvailable(board, i, j, isblack))
				available_list.push_back(i * rank_n + j);
	int result = available_list[rand() % available_list.size()];
	return { result / rank_n, result % rank_n };
};
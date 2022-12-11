#include <functional>
#include <windows.h>
#include <iostream>
#include <conio.h>
#include <string>
#include <format>

#include "pair.hpp"
#include "game.hpp"

using namespace std;

#define CSI "\x1b["
#define ERASE CSI "X"
#define FORWARD CSI "C"

#define COLOR CSI "38;5;231m"
#define NEGATIVE CSI "0m" CSI "38;5;231;7m"

#define BLINK CSI "5m"
#define NOBLINK CSI "25m"

int getwch_noblock() {
	return _kbhit() ? _getwch() : -1;
}

string repeat(const function<string(int)>&& genf, int times) {
	std::ostringstream os;
	for (int i = 0; i < times; i++)
		os << genf(i);
	return os.str();
}
string repeat(string word, int times) {
	return repeat([word](auto _) { return word; }, times);
}

constexpr int uninited = -2;

using PIndex = pair<int, char>;

struct BoardPrinter
{
	int rank_n;
	Pair cell; // TODO diverse when rank_n >= 10, otherwise table_char will error.
	Pair screen_size;
	Pair board_size, table_size;
	Pair board_corner, table_corner;

	BoardPrinter(int rank_n, Pair screen_size) : rank_n(rank_n), screen_size(screen_size),
		cell(Pair{ 2, 4 }),
		board_size(cell* (rank_n - 1) + 1),
		board_corner((screen_size - board_size) / 2),
		table_size(board_size + Pair{ 2 * 2,  4 * 2 }),
		table_corner((screen_size - table_size) / 2) {}

	void print()
	{
		// Enter the alternate buffer
		printf(CSI "?1049h");

		printf(CSI "?12"); // Stop blinking the cursor

		// Clear screen, tab stops, set, stop at columns 16, 32
		printf(CSI "1;1H");
		printf(CSI "2J"); // Clear screen

		// Set scrolling margins to 3, h-2
		printf(CSI "3;%dr", screen_size.y - 2);

		printf(CSI "?25l"); // Hide the cursor

		print_title();
		print_panel();
		draw_table();
		draw_board();
	}

	void print_title()
	{
		const string game_name = "  NoGo";
		string str = "New Game *"; // TODO
		auto i = (screen_size.y - str.size()) / 2;

		string output(screen_size.y, ' ');
		copy(game_name.begin(), game_name.end(), output.begin());
		copy(str.begin(), str.end(), output.begin() + i);

		Pair::print({ 1, 1 }, NEGATIVE, output);
	}

	const string str2 = "Stone Position to Play on Board:";

	void print_panel()
	{
		string str = "[ Welcome to NoGo.  For basic help, type Ctrl+G. ]";
		Pair p{ screen_size.x - 2, (screen_size.y - (int)str.size()) / 2 };
		Pair::print(p, str);
		p = { screen_size.x - 1, 1 };

		Pair::print(p, str2 + repeat(" ", screen_size.y - (int)str2.size()));


		//TODO
	}

	void draw_table()
	{
		Pair p = table_corner;
		Pair::println(p, "╔" + repeat("═", table_size.y - 2) + "╗");
		for (int i = 2; i < table_size.x; i++)
			Pair::println(p, "║" + string(table_size.y - 2, ' ') + "║");
		Pair::println(p, "╚" + repeat("═", table_size.y - 2) + "╝");

		p = table_corner + Pair{ 1, 4 };
		Pair::print(p, repeat([&](auto i) { return i % cell.y == 0 ? string(1, 'A' + i / cell.y) : " "; }, board_size.y));
		p = table_corner + Pair{ 2, 2 };
		for (int i = 0; i < board_size.x; i++)
			Pair::println(p, i % cell.x == 0 ? to_string(rank_n - i / cell.x) : " ");
	}

	string table_char(Pair pos)
	{
		if (pos.x < 0 || pos.x >= board_size.x || pos.y < 0 || pos.y >= board_size.y) return " ";

		if (pos.x == 0 && pos.y == 0) return "┏";
		if (pos.x == 0 && pos.y == board_size.y - 1) return "┓";
		if (pos.x == 0 && pos.y % cell.y == 0) return "┯";
		if (pos.x == board_size.x - 1 && pos.y == 0) return "┗";
		if (pos.x == board_size.x - 1 && pos.y == board_size.y - 1) return "┛";
		if (pos.x == board_size.x - 1 && pos.y % cell.y == 0) return "┷";
		if (pos.x == 0 || pos.x == board_size.x - 1) return "━";

		if (pos.y == 0 && pos.x % cell.y == 0) return "┠";
		if (pos.y == board_size.y - 1 && pos.x % cell.y == 0) return "┨";
		if (pos.y == 0 || pos.y == board_size.y - 1) return "┃";

		if (pos.x % cell.x == 0 && pos.y % cell.y == 0) return "┼";
		if (pos.x % cell.x == 0) return "─";
		if (pos.y % cell.y == 0) return "│";

		return " ";
	}

	void draw_board()
	{
		Pair p = board_corner;
		for (int i = 0; i < board_size.x; i++)
			Pair::println(p, repeat([&](auto j) { return table_char({ i, j }); }, board_size.y));
	}

	void update_board(const Contest::BoardType& board) {
		for (auto i = 0; i < rank_n; i++) {
			for (auto j = 0; j < rank_n; j++) {
				if (!board[i][j]) continue;
				Pair::print(board_corner + Pair{ i, j } *cell - Pair{ 0, 1 },
					COLOR, ERASE, " ", ERASE, stonec[board[i][j] == 1], ERASE, " ");
			}
		} // TODO lazy update
	}

	static inline char stonec[][3]{ "○", "●" };
	//🔘⦾⦿○●⚪◦⬤

	Pair from_index(PIndex index)
	{
		auto [digit, alpha] = index;
		return { digit != uninited ? rank_n - digit : uninited,
				 alpha != uninited ? index.second - 'A' : uninited };
	}
	PIndex to_index(Pair pos)
	{
		return { pos.x != uninited ? rank_n - pos.x : uninited,
				 pos.y != uninited ? 'A' + pos.y : uninited };
	}
	bool stone_pos_valid(Pair pos)
	{
		if ((pos.x < 0 || pos.x >= rank_n) && pos.x != uninited) return false;
		if ((pos.y < 0 || pos.y >= rank_n) && pos.y != uninited) return false;
		// TODO check if is occupied
		return true;
	}
	void echo_candidate(PIndex index)
	{
		auto [digit, alpha] = index;
		string sdigit = digit != uninited ? to_string(digit) : "  ",
			   salpha = alpha != uninited ? string(1, alpha) : " ";

		Pair::print({ screen_size.x - 1, (int)str2.size() + 2 },
			salpha + sdigit);
	}

	string blink_mode(string str) const
	{
		return string(BLINK) + str + string(NOBLINK);
	}
	void index_blink(Pair pos, bool flag)
	{
		auto [digit, alpha] = to_index(pos);
		string sdigit = to_string(digit),
			   salpha = string(1, alpha);
		if (flag) sdigit = blink_mode(sdigit),
				  salpha = blink_mode(salpha);

		if (pos.x >= 0 && pos.x < rank_n)
			Pair::print(table_corner + Pair{ 2, 2 } + Pair{ pos.x * cell.x, 0 },
				COLOR, ERASE, sdigit);
		if (pos.y >= 0 && pos.y < rank_n)
			Pair::print(table_corner + Pair{ 1, 4 } + Pair{ 0, pos.y * cell.y },
				COLOR, ERASE, salpha);
	}

	void stone_blink(Pair pos, bool isblack, bool flag)
	{
		if (!stone_pos_valid(pos) || pos.x == uninited || pos.y == uninited)
			return;
		pos = pos * cell - Pair{ 0, 1 };
		string s1 = flag ? string(ERASE) + " " : table_char(pos),
			s2 = flag ? string(ERASE) + blink_mode(stonec[isblack]) : table_char(pos + Pair{ 0, 1 }),
			s3 = flag ? string(ERASE) + " " : table_char(pos + Pair{0, 2});
		Pair::print(board_corner + pos, COLOR, s1, s2, s3);
	}

	Pair update_candidate(Pair pos, Pair newpos, bool isblack)
	{
		if (!stone_pos_valid(newpos)) return pos;
		index_blink(pos, false);
		stone_blink(pos, isblack, false);

		echo_candidate(to_index(newpos));
		index_blink(newpos, true);
		stone_blink(newpos, isblack, true);
		return newpos;
	}
	Pair update_candidate(Pair pos, PIndex nindex, bool isblack)
	{
		return update_candidate(pos, from_index(nindex), isblack);
	}
};

void crash(string error_message)
{
	cout << error_message << endl;
	exit(-1);
}

int main()
{
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE), hIn = GetStdHandle(STD_INPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE || hIn == INVALID_HANDLE_VALUE) crash("Couldn't get the console handle. Quitting.");
	DWORD out_mode = 0, in_mode = 0;
	if (!GetConsoleMode(hOut, &out_mode) || !GetConsoleMode(hIn, &in_mode)) crash("Unable to enter VT processing mode. Quitting.");
	out_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING, in_mode |= ENABLE_VIRTUAL_TERMINAL_INPUT;
	if (!SetConsoleMode(hOut, out_mode) || !SetConsoleMode(hIn, in_mode)) crash("Unable to enter VT processing mode. Quitting.");

	CONSOLE_SCREEN_BUFFER_INFO ScreenBufferInfo;
	GetConsoleScreenBufferInfo(hOut, &ScreenBufferInfo);
	Pair screen_size = { ScreenBufferInfo.srWindow.Bottom - ScreenBufferInfo.srWindow.Top + 1,
						 ScreenBufferInfo.srWindow.Right - ScreenBufferInfo.srWindow.Left + 1 };
	// TODO X and Y meaning

	BoardPrinter printer(10, screen_size);
	printer.print();

	auto work = [&printer](auto board, auto isblack) -> Pair {
		Pair pos{uninited, uninited};
		PIndex index{uninited, uninited};
		auto& [digit, alpha] = index;
		while (true) {
			Sleep(500);
			auto wch = getwch_noblock();

			Pair delta[] = {
				{-1, 0}, {+1, 0}, {0, +1}, {0, -1}
				//  Up, Down, Right, Left
				//  ESC A, ESC B, ESC C, ESC D
			};
			if (wch == 0x1b && _getwch() == '[' &&
				(wch = _getwch(), 'A' <= wch && wch <= 'D')) {
				int i = wch - 'A';
				if (pos.x == uninited && delta[i].x ||
					pos.y == uninited && delta[i].y) continue;
				Pair new_pos = pos + delta[i];
				while (printer.stone_pos_valid(new_pos) && board[new_pos.x][new_pos.y])
					new_pos += delta[i];
				pos = printer.update_candidate(pos, new_pos, isblack);
			}
			else if (wch == 27) {
				return { -1, -1 }; // TODO about self-killing, and normally exit the contest
			}
			else if (wch == 0x7f) {
				if (digit) digit /= 10;
				else alpha = 0;
				pos = printer.update_candidate(pos, index, isblack);
			}
			else if (isdigit(wch)) {
				//digit = 10 * digit + wch - '0';
				digit = wch - '0';
				pos = printer.update_candidate(pos, index, isblack);
				index = printer.to_index(pos);
			}
			else if (isalpha(wch)) {
				alpha = toupper(wch);
				pos = printer.update_candidate(pos, index, isblack);
				index = printer.to_index(pos);
			}
			else if (wch == '\r') {
				printer.index_blink(pos, false);
				printer.echo_candidate({ uninited, uninited });
				return pos;
			}
		}
	};
	Contest contest(printer.rank_n, work, bot_player);
	while (contest.play()) {
		printer.update_board(contest.board);
	}

	wchar_t wch = _getwch();

	// Exit the alternate buffer
	printf(CSI "?1049");
}

// TODO screen size change

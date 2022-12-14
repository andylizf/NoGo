#include <conio.h>
#include <windows.h>

#include <format>
#include <functional>
#include <iostream>
#include <ranges>
#include <string>

#include "bot.hpp"
#include "game.hpp"
#include "pair.hpp"

using namespace std;
namespace fs = std::filesystem;
namespace ranges = std::ranges;

#define CSI "\x1b["
#define ERASE CSI "X"
#define FORWARD CSI "C"

#define COLOR CSI "38;5;231;7m"
#define NEGATIVE CSI "27m"
#define NONEGATIVE CSI "7m"

#define BLINK CSI "5m"
#define NOBLINK CSI "25m"

int getch_noblock()
{
    return _kbhit() ? _getch() : -1;
}

string repeat(const function<string(int)>&& genf, int times)
{
    return ranges::fold_left(
        views::iota(0, times) | views::transform([genf](int i) {
            return genf(i);
        }),
        "", std::plus {});
}
string repeat(string word, int times)
{
    return repeat([word](auto _) { return word; }, times);
}

string current_time()
{
    auto t = time(nullptr);
    stringstream ss;
    ss << put_time(localtime(&t), "%F_%T"); // ISO 8601 without timezone information.
    auto s = ss.str();
    replace(s.begin(), s.end(), ':', '-');
    return s;
}

struct BoardPrinter {
    Pair cell;
    Pair screen_size;
    Pair board_size, table_size;
    Pair board_corner, table_corner;

    BoardPrinter(Pair screen_size)
        : screen_size(screen_size)
        , cell { 2, 4 }
        , board_size(cell * (rank_n - 1) + 1)
        , board_corner((screen_size - board_size) / 2)
        , table_size(board_size + Pair { 2, 4 } * 2)
        , table_corner((screen_size - table_size) / 2)
    {
    }

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

        Pair::print({ 1, 1 }, COLOR, output);
    }

    const string str2 = "Stone Position to Play on Board:";

    void print_empty_line(int line_num)
    {
        Pair::print({ line_num, 1 }, repeat(" ", screen_size.y));
    }
    string negative_mode(string str) const
    {
        return string(NEGATIVE) + str + string(NONEGATIVE);
    }
    int column_width;

    void print_banner(string str)
    {
        Pair p { screen_size.x - 3, (screen_size.y - (int)str.size()) / 2 };
        print_empty_line_negative(p.x);
        Pair::print(p, str);
    }
    void print_dialog_title(string title)
    {
        Pos p { screen_size.x - 2, 1 };
        print_empty_line(p.x);
        Pair::print(p, title);
    }
    void print_panel()
    {
        print_banner("[ Welcome to NoGo.  For basic help, type Ctrl+G. ]");
        print_dialog_title(str2);

        array<string, 8> options {
            "^G" + negative_mode(" Help"), "^S" + negative_mode(" Save"),
            "^H" + negative_mode(" Hint"), "^Z" + negative_mode(" Undo"),
            "^X" + negative_mode(" Exit"), "^O" + negative_mode(" Load"),
            "^R" + negative_mode(" Replay"), "^Y" + negative_mode(" Redo")
        };

        column_width = screen_size.y / 4;

        Pair p = { screen_size.x - 1, 1 };
        print_empty_line_negative(p.x);
        for (const auto& option : vector(options.begin(), options.begin() + 4))
            Pair::print(p, option), p.y += column_width;

        p = { screen_size.x, 1 };
        print_empty_line_negative(p.x);
        for (const auto& option : vector(options.begin() + 4, options.begin() + 8))
            Pair::print(p, option), p.y += column_width;
    }

    void print_empty_line_negative(int line_num)
    {
        Pair::print({ line_num, 1 }, repeat(negative_mode(" "), screen_size.y));
    }

    void print_dialog(string title, function<void()> yes, function<void()> no)
    {
        print_dialog_title(title);
        array<string, 3> options {
            " Y" + negative_mode(" Yes"),
            " N" + negative_mode(" No"),
            "^C" + negative_mode(" Cancel")
        };
        Pair p = { screen_size.x - 1, 1 };
        print_empty_line_negative(p.x);
        Pair::print(p, options[0]);
        p.x++;
        print_empty_line_negative(p.x);
        Pair::print(p, options[1]);
        p.y += column_width;
        Pair::print(p, options[2]);
        while (true) {
            Sleep(500);
            auto c = getch_noblock();
            if (c == -1) {
                continue;
            } else if (toupper(c) == 'Y') {
                yes();
            } else if (toupper(c) == 'N') {
                no();
            } else if (c == 3) { // CTRL + C
                print_panel();
                return;
            }
        }
    }

    void refresh_input_buffer(Pair p)
    {
        Pair::print(p, repeat(" ", screen_size.y - p.y));
    }

    void print_input_dialog(string title, function<void(string)> enter)
    {
        print_dialog_title(title);
        array<string, 2> options {
            "^G" + negative_mode(" Help"),
            "^C" + negative_mode(" Cancel")
        };
        Pair p = { screen_size.x - 1, 1 };
        print_empty_line_negative(p.x);
        Pair::print(p, options[0]);
        p.x++;
        print_empty_line_negative(p.x);
        Pair::print(p, options[1]);

        p = { screen_size.x - 2, (int)title.size() + 2 };
        string str {};
        while (true) {
            Sleep(500);
            auto c = getch_noblock();
            if (c == -1) {
                continue;
            } else if (isdigit(c) || isalpha(c)) {
                str += c;
                refresh_input_buffer(p);
                Pair::print(p, str);
            } else if (c == 0x7f) {
                if (str.size())
                    str.pop_back();
                refresh_input_buffer(p);
                Pair::print(p, str);
            } else if (c == '\r') {
                enter(str);
            }
        }
    }

    void save_file(Contest& contest)
    {
        print_dialog(
            "Save modified situation? ", [&contest]() {
            string file_name = "situation_" + current_time() + ".nogo";
            contest.save(file_name);
            exit(0); }, [] { exit(0); });
    }

    string str5 = "Situation File to Load: ";
    void read_file(Contest& contest, BoardPrinter& printer)
    {
        print_input_dialog(str5, [&contest, &printer](auto filename) {
            contest.current = {};
            contest.load(filename + ".nogo");
            printer.update_board(contest.current.board);
            printer.print_panel();
        });
    }

    void draw_table()
    {
        Pair p = table_corner;
        Pair::println(p, "╔" + repeat("═", table_size.y - 2) + "╗");
        for (int i = 2; i < table_size.x; i++)
            Pair::println(p, "║" + string(table_size.y - 2, ' ') + "║");
        Pair::println(p, "╚" + repeat("═", table_size.y - 2) + "╝");

        p = table_corner + Pair { 1, 4 };
        Pair::print(p, repeat([&](auto i) {
            return i % cell.y == 0 ? string(1, 'A' + i / cell.y) : " ";
        },
                           board_size.y));
        p = table_corner + Pair { 2, 2 };
        for (int i = 0; i < board_size.x; i++)
            Pair::println(p, i % cell.x == 0 ? to_string(rank_n - i / cell.x) : " ");
    }

    string table_char(Pair p)
    {
        if (p.x < 0 || p.x >= board_size.x || p.y < 0 || p.y >= board_size.y)
            return " ";

        if (p.x == 0 && p.y == 0)
            return "┏";
        if (p.x == 0 && p.y == board_size.y - 1)
            return "┓";
        if (p.x == 0 && p.y % cell.y == 0)
            return "┯";
        if (p.x == board_size.x - 1 && p.y == 0)
            return "┗";
        if (p.x == board_size.x - 1 && p.y == board_size.y - 1)
            return "┛";
        if (p.x == board_size.x - 1 && p.y % cell.y == 0)
            return "┷";
        if (p.x == 0 || p.x == board_size.x - 1)
            return "━";

        if (p.y == 0 && p.x % cell.y == 0)
            return "┠";
        if (p.y == board_size.y - 1 && p.x % cell.y == 0)
            return "┨";
        if (p.y == 0 || p.y == board_size.y - 1)
            return "┃";

        if (p.x % cell.x == 0 && p.y % cell.y == 0)
            return "┼";
        if (p.x % cell.x == 0)
            return "─";
        if (p.y % cell.y == 0)
            return "│";

        return " ";
    }

    void draw_board()
    {
        Pair p = board_corner;
        for (int i = 0; i < board_size.x; i++)
            Pair::println(p, repeat([&](auto j) {
                return table_char({ i, j });
            },
                                 board_size.y));
    }
    void update_board(const BoardType& board)
    {
        for (Pos p : board.index()) {
            bool flag = board[p], isblack = board[p] == 1; // TODO
            Pos p2 = p * cell - Pair { 0, 1 };
            string s1 = flag ? string(ERASE) + " " : table_char(p2),
                   s2 = flag ? string(ERASE) + stonec[isblack]
                             : table_char(p2 + Pair { 0, 1 }),
                   s3 = flag ? string(ERASE) + " " : table_char(p2 + Pair { 0, 2 });
            Pair::print(board_corner + p2, COLOR, s1, s2, s3);
        } // TODO lazy update
    }

    static inline char stonec[][3] { "○", "●" };
    // 🔘⦾⦿○●⚪◦⬤

    bool stone_p_valid(Pos p)
    {
        if ((p.x < 0 || p.x >= rank_n) && p.x != Pos::uninited)
            return false;
        if ((p.y < 0 || p.y >= rank_n) && p.y != Pos::uninited)
            return false;
        // TODO check if is occupied
        return true;
    }
    void echo_candidate(Pos p)
    {
        Pair::print({ screen_size.x - 2, (int)str2.size() + 2 }, p);
    }

    string blink_mode(string str) const
    {
        return string(BLINK) + str + string(NOBLINK);
    }
    void index_blink(Pos p, bool flag)
    {
        string sdigit = string(1, p.get_digit()), salpha = string(1, p.get_alpha());
        if (flag)
            sdigit = blink_mode(sdigit), salpha = blink_mode(salpha);

        if (p.x >= 0 && p.x < rank_n)
            Pair::print(table_corner + Pair { 2, 2 } + Pair { p.x * cell.x, 0 }, COLOR,
                ERASE, sdigit);
        if (p.y >= 0 && p.y < rank_n)
            Pair::print(table_corner + Pair { 1, 4 } + Pair { 0, p.y * cell.y }, COLOR,
                ERASE, salpha);
    }

    void stone_blink(Pos p, bool isblack, bool flag)
    {
        if (!stone_p_valid(p) || p.x == Pos::uninited || p.y == Pos::uninited)
            return;
        p = p * cell - Pair { 0, 1 };
        string s1 = flag ? string(ERASE) + " " : table_char(p),
               s2 = flag ? string(ERASE) + blink_mode(stonec[isblack])
                         : table_char(p + Pair { 0, 1 }),
               s3 = flag ? string(ERASE) + " " : table_char(p + Pair { 0, 2 });
        Pair::print(board_corner + p, COLOR, s1, s2, s3);
    }

    Pos update_candidate(Pos p, Pos newp, bool isblack)
    {
        if (!stone_p_valid(newp))
            return p;

        index_blink(p, false);
        stone_blink(p, isblack, false);

        echo_candidate(newp);
        index_blink(newp, true);
        stone_blink(newp, isblack, true);
        return newp;
    }
};

void crash(string error_message)
{
    cout << error_message << endl;
    exit(-1);
}

#ifdef NOGO
int main()
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE),
           hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE || hIn == INVALID_HANDLE_VALUE)
        crash("Couldn't get the console handle. Quitting.");
    DWORD out_mode = 0, in_mode = 0;
    if (!GetConsoleMode(hOut, &out_mode) || !GetConsoleMode(hIn, &in_mode))
        crash("Unable to enter VT processing mode. Quitting.");
    out_mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING,
        in_mode |= ENABLE_VIRTUAL_TERMINAL_INPUT, in_mode &= ~ENABLE_PROCESSED_INPUT;
    if (!SetConsoleMode(hOut, out_mode) || !SetConsoleMode(hIn, in_mode))
        crash("Unable to enter VT processing mode. Quitting.");

    CONSOLE_SCREEN_BUFFER_INFO ScreenBufferInfo;
    GetConsoleScreenBufferInfo(hOut, &ScreenBufferInfo);
    Pair screen_size = {
        ScreenBufferInfo.srWindow.Bottom - ScreenBufferInfo.srWindow.Top + 1,
        ScreenBufferInfo.srWindow.Right - ScreenBufferInfo.srWindow.Left + 1
    };
    // TODO X and Y meaning

    BoardPrinter printer(screen_size);
    printer.print();

    Pos p {};
    auto work = [&p](auto) -> Pos { return p; };

    Contest contest(work, mcts_bot_player);

    BoardType& board = contest.current.board;
    bool isblack = true;

    while (true) {
        Sleep(500);
        auto c = getch_noblock();

        Pos delta[] {
            { -1, 0 }, { +1, 0 }, { 0, +1 }, { 0, -1 }
            //  Up, Down, Right, Left
            //  ESC A, ESC B, ESC C, ESC D
        };
        if (c == 0x1b && _getwch() == '[' && (c = _getwch(), 'A' <= c && c <= 'D')) {
            int i { c - 'A' };
            if (p.x == Pos::uninited && delta[i].x || p.y == Pos::uninited && delta[i].y)
                continue;
            Pos newp = p + delta[i];
            if (p.x != Pos::uninited && p.y != Pos::uninited)
                while (printer.stone_p_valid(newp) && board[newp])
                    newp += delta[i];
            p = printer.update_candidate(p, newp, isblack);
        } else if (c == 0x7f) {
            Pos newp = p;
            if (p.y != Pos::uninited)
                newp.set_alpha(' ');
            else
                newp.set_digit(' ');
            p = printer.update_candidate(p, newp, isblack);
        } else if (isdigit(c)) {
            Pos newp = p;
            newp.set_digit(c);
            p = printer.update_candidate(p, newp, isblack);
        } else if (isalpha(c)) {
            Pos newp = p;
            newp.set_alpha(toupper(c));
            p = printer.update_candidate(p, newp, isblack);
        } else if (c == '\r') {
            printer.index_blink(p, false);
            if (!contest.play()) {
                printer.print_banner(format(" Game ends. Player {} wins! ", contest.winner == 1 ? "black" : "white"));
                // TODO
            }
            printer.echo_candidate(p = Pos {});
            if (!contest.play()) {
                printer.print_banner(format(" Game ends. Player {} wins! ", contest.winner == 1 ? "black" : "white"));
            }
            printer.update_board(board);
        } else if (c == 24) { // exit
            if (!contest.round())
                exit(0);
            printer.save_file(contest);
        } else if (c == 15) { // load
            printer.read_file(contest, printer);
        } else if (c == 19) { // save
            string file_name = "situation_" + current_time() + ".nogo";
            contest.save(file_name);
        } else if (c == 7) { // help

        } else if (c == 18) { // replay

        } else if (c == 8) { // hint
            Pair newp = mcts_bot_player(contest.current);
            p = printer.update_candidate(p, newp, isblack);
        } else if (c == 26) { // undo
            if (contest.round() < 2)
                continue;
            auto newp = (contest.current.revoke(), contest.current.revoke());
            printer.update_board(board);
            p = printer.update_candidate(p, newp, isblack);
        }
    }

    wchar_t c = _getwch();

    // Exit the alternate buffer
    printf(CSI "?1049");
}

// TODO screen size change

#endif
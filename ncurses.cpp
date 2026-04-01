#include <ncurses.h>

#include <cstdio>
#include <cstdlib>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace
{

	constexpr int Minimum_board_size = 3;
	constexpr int Maximum_board_size = 15;
	constexpr int cell_size = 4;

	struct Score
	{
		int x_wins = 0;
		int o_wins = 0;
		int draws = 0;
	};

	enum class RoundAction
	{
		kNextRound,
		kChangeBoardSize,
		kQuit,
	};

	void InitColors()
	{
		if (!has_colors())
		{
			return;
		}

		start_color();
		use_default_colors();

		init_pair(1, COLOR_CYAN, -1);						 // Title
		init_pair(2, COLOR_RED, -1);						 // X
		init_pair(3, COLOR_GREEN, -1);					 // O
		init_pair(4, COLOR_BLACK, COLOR_YELLOW); // Cursor
		init_pair(5, COLOR_BLUE, -1);						 // Grid
		init_pair(6, COLOR_WHITE, -1);					 // Panel text
		init_pair(7, COLOR_BLACK, COLOR_GREEN);	 // Winning cells
	}

	void DrawCenteredText(int y, const std::string &text, int color_pair = 0, int attrs = 0)
	{
		int x = (COLS - static_cast<int>(text.size())) / 2;
		if (x < 0)
		{
			x = 0;
		}

		if (color_pair > 0 && has_colors())
		{
			attron(COLOR_PAIR(color_pair));
		}
		if (attrs != 0)
		{
			attron(attrs);
		}

		mvprintw(y, x, "%s", text.c_str());

		if (attrs != 0)
		{
			attroff(attrs);
		}
		if (color_pair > 0 && has_colors())
		{
			attroff(COLOR_PAIR(color_pair));
		}
	}

	void DrawTopBanner()
	{
		if (has_colors())
		{
			attron(COLOR_PAIR(1) | A_BOLD);
		}
		DrawCenteredText(0, "TIC TAC TOE", 1, A_BOLD);
		DrawCenteredText(1, "ncurses terminal game", 1);
		if (has_colors())
		{
			attroff(COLOR_PAIR(1) | A_BOLD);
		}
	}

	bool IsBoardFull(const std::vector<std::vector<char>> &board)
	{
		const int size = static_cast<int>(board.size());
		for (int row = 0; row < size; ++row)
		{
			for (int col = 0; col < size; ++col)
			{
				if (board[row][col] == ' ')
				{
					return false;
				}
			}
		}
		return true;
	}

	std::vector<std::pair<int, int>> GetWinningCells(const std::vector<std::vector<char>> &board, char *winner_out)
	{
		const int size = static_cast<int>(board.size());
		std::vector<std::pair<int, int>> cells;

		for (int row = 0; row < size; ++row)
		{
			char first = board[row][0];
			if (first == ' ')
			{
				continue;
			}

			bool match = true;
			for (int col = 1; col < size; ++col)
			{
				if (board[row][col] != first)
				{
					match = false;
					break;
				}
			}

			if (match)
			{
				*winner_out = first;
				for (int col = 0; col < size; ++col)
				{
					cells.push_back({row, col});
				}
				return cells;
			}
		}

		for (int col = 0; col < size; ++col)
		{
			char first = board[0][col];
			if (first == ' ')
			{
				continue;
			}

			bool match = true;
			for (int row = 1; row < size; ++row)
			{
				if (board[row][col] != first)
				{
					match = false;
					break;
				}
			}

			if (match)
			{
				*winner_out = first;
				for (int row = 0; row < size; ++row)
				{
					cells.push_back({row, col});
				}
				return cells;
			}
		}

		char main_diag = board[0][0];
		if (main_diag != ' ')
		{
			bool match = true;
			for (int i = 1; i < size; ++i)
			{
				if (board[i][i] != main_diag)
				{
					match = false;
					break;
				}
			}

			if (match)
			{
				*winner_out = main_diag;
				for (int i = 0; i < size; ++i)
				{
					cells.push_back({i, i});
				}
				return cells;
			}
		}

		char anti_diag = board[0][size - 1];
		if (anti_diag != ' ')
		{
			bool match = true;
			for (int i = 1; i < size; ++i)
			{
				if (board[i][size - 1 - i] != anti_diag)
				{
					match = false;
					break;
				}
			}

			if (match)
			{
				*winner_out = anti_diag;
				for (int i = 0; i < size; ++i)
				{
					cells.push_back({i, size - 1 - i});
				}
				return cells;
			}
		}

		*winner_out = ' ';
		return cells;
	}

	bool FitsInTerminal(int board_size)
	{
		const int board_height = board_size * 2 + 1;
		const int board_width = board_size * cell_size + 1;
		const int min_height = board_height + 12;
		const int min_width = board_width + 14;
		return (LINES >= min_height && COLS >= min_width);
	}

	void DrawBoardGrid(int board_size, int origin_y, int origin_x)
	{
		const int board_height = board_size * 2 + 1;
		const int board_width = board_size * cell_size + 1;

		if (has_colors())
		{
			attron(COLOR_PAIR(5));
		}

		for (int row = 0; row <= board_size; ++row)
		{
			int y = origin_y + row * 2;
			for (int x = 0; x < board_width; ++x)
			{
				mvaddch(y, origin_x + x, (x % cell_size == 0) ? ACS_PLUS : ACS_HLINE);
			}
		}

		for (int col = 0; col <= board_size; ++col)
		{
			int x = origin_x + col * cell_size;
			for (int y = 0; y < board_height; ++y)
			{
				mvaddch(origin_y + y, x, (y % 2 == 0) ? ACS_PLUS : ACS_VLINE);
			}
		}

		if (has_colors())
		{
			attroff(COLOR_PAIR(5));
		}
	}

	void DrawBoard(const std::vector<std::vector<char>> &board,
								 int board_size,
								 int origin_y,
								 int origin_x,
								 int cursor_row,
								 int cursor_col,
								 char current_player,
								 const Score &score,
								 const std::string &status,
								 const std::set<std::pair<int, int>> &highlighted)
	{
		clear();
		DrawTopBanner();

		if (has_colors())
		{
			attron(COLOR_PAIR(6));
		}
		mvprintw(3, 2, "Board size: %dx%d", board_size, board_size);
		mvprintw(4, 2, "Score  X:%d  O:%d  Draw:%d", score.x_wins, score.o_wins, score.draws);
		mvprintw(5, 2, "Controls: arrows/mouse + Enter | u undo | n new round | b board size | r reset score | q quit");
		mvprintw(6, 2, "Turn: %c", current_player);
		if (has_colors())
		{
			attroff(COLOR_PAIR(6));
		}

		DrawBoardGrid(board_size, origin_y, origin_x);

		for (int row = 0; row < board_size; ++row)
		{
			for (int col = 0; col < board_size; ++col)
			{
				const bool is_cursor = (row == cursor_row && col == cursor_col);
				const bool is_highlighted = highlighted.find({row, col}) != highlighted.end();
				const int y = origin_y + row * 2 + 1;
				const int x = origin_x + col * cell_size + 1;

				if (is_highlighted && has_colors())
				{
					attron(COLOR_PAIR(7) | A_BOLD);
				}
				else if (is_cursor && has_colors())
				{
					attron(COLOR_PAIR(4) | A_BOLD);
				}
				else if (is_cursor)
				{
					attron(A_REVERSE);
				}

				if (board[row][col] == 'X' && has_colors())
				{
					attron(COLOR_PAIR(2) | A_BOLD);
				}
				else if (board[row][col] == 'O' && has_colors())
				{
					attron(COLOR_PAIR(3) | A_BOLD);
				}

				mvprintw(y, x, " %c ", board[row][col]);

				if (board[row][col] == 'X' && has_colors())
				{
					attroff(COLOR_PAIR(2) | A_BOLD);
				}
				else if (board[row][col] == 'O' && has_colors())
				{
					attroff(COLOR_PAIR(3) | A_BOLD);
				}

				if (is_highlighted && has_colors())
				{
					attroff(COLOR_PAIR(7) | A_BOLD);
				}
				else if (is_cursor && has_colors())
				{
					attroff(COLOR_PAIR(4) | A_BOLD);
				}
				else if (is_cursor)
				{
					attroff(A_REVERSE);
				}
			}
		}

		DrawCenteredText(LINES - 2, status, 6, A_BOLD);
		refresh();
	}

	bool MouseToCell(int mouse_y, int mouse_x, int board_size, int origin_y, int origin_x,
									 int *row_out, int *col_out)
	{
		int relative_y = mouse_y - origin_y;
		int relative_x = mouse_x - origin_x;

		const int board_height = board_size * 2 + 1;
		const int board_width = board_size * cell_size + 1;

		if (relative_y <= 0 || relative_x <= 0 || relative_y >= board_height - 1 || relative_x >= board_width - 1)
		{
			return false;
		}

		if (relative_y % 2 == 0)
		{
			return false;
		}

		if (relative_x % cell_size == 0)
		{
			return false;
		}

		int row = (relative_y - 1) / 2;
		int col = (relative_x - 1) / cell_size;

		if (row < 0 || row >= board_size || col < 0 || col >= board_size)
		{
			return false;
		}

		*row_out = row;
		*col_out = col;
		return true;
	}

	int PromptBoardSize()
	{
		int size = Minimum_board_size;
		while (true)
		{
			clear();
			DrawTopBanner();
			DrawCenteredText(4, "Choose board size", 6, A_BOLD);
			DrawCenteredText(6, "Left/Right: change size", 6);
			DrawCenteredText(7, "Enter: start game", 6);
			DrawCenteredText(8, "q: quit", 6);

			std::string size_text = "[ " + std::to_string(size) + " x " + std::to_string(size) + " ]";
			DrawCenteredText(10, size_text, 1, A_BOLD | A_UNDERLINE);

			const int needed_h = size * 2 + 12;
			const int needed_w = size * cell_size + 14;
			std::string fit_text = (LINES >= needed_h && COLS >= needed_w)
																 ? "Terminal size is sufficient"
																 : "Terminal too small for this board size";
			DrawCenteredText(12, fit_text, (LINES >= needed_h && COLS >= needed_w) ? 3 : 2, A_BOLD);

			refresh();

			int ch = getch();
			if (ch == 'q' || ch == 'Q')
			{
				return 0;
			}

			if (ch == KEY_LEFT && size > Minimum_board_size)
			{
				--size;
			}
			else if (ch == KEY_RIGHT && size < Maximum_board_size)
			{
				++size;
			}
			else if (ch == '\n' || ch == KEY_ENTER || ch == 10 || ch == 13)
			{
				if (FitsInTerminal(size))
				{
					return size;
				}
			}
		}
	}

	RoundAction RunSingleGame(int board_size, Score *score)
	{
		std::vector<std::vector<char>> board(board_size, std::vector<char>(board_size, ' '));
		std::vector<std::pair<int, int>> move_history;
		char current_player = 'X';
		int cursor_row = 0;
		int cursor_col = 0;

		const int board_height = board_size * 2 + 1;
		const int board_width = board_size * cell_size + 1;
		const int origin_y = (LINES - board_height) / 2 + 2;
		const int origin_x = (COLS - board_width) / 2;

		bool round_over = false;
		char winner = ' ';
		std::set<std::pair<int, int>> highlighted;
		std::string status = "Make a move";

		while (true)
		{
			if (!FitsInTerminal(board_size))
			{
				clear();
				DrawTopBanner();
				DrawCenteredText(5, "The Terminal is too small for this board.", 2, A_BOLD);
				DrawCenteredText(7, "To change the size of the board and press any key to return to board menu.", 6);
				refresh();
				getch();
				return RoundAction::kChangeBoardSize;
			}

			DrawBoard(board, board_size, origin_y, origin_x, cursor_row, cursor_col,
								current_player, *score, status, highlighted);

			int ch = getch();
			if (ch == 'q' || ch == 'Q')
			{
				return RoundAction::kQuit;
			}
			if (ch == 'b' || ch == 'B')
			{
				return RoundAction::kChangeBoardSize;
			}
			if (ch == 'r' || ch == 'R')
			{
				score->x_wins = 0;
				score->o_wins = 0;
				score->draws = 0;
				status = "Score reset";
				continue;
			}
			if (ch == 'n' || ch == 'N')
			{
				return RoundAction::kNextRound;
			}

			if (round_over)
			{
				status = "Round ended. Press n, b, r, or q";
				continue;
			}

			if (ch == KEY_UP && cursor_row > 0)
			{
				--cursor_row;
				continue;
			}
			if (ch == KEY_DOWN && cursor_row < board_size - 1)
			{
				++cursor_row;
				continue;
			}
			if (ch == KEY_LEFT && cursor_col > 0)
			{
				--cursor_col;
				continue;
			}
			if (ch == KEY_RIGHT && cursor_col < board_size - 1)
			{
				++cursor_col;
				continue;
			}

			if (ch == 'u' || ch == 'U')
			{
				if (move_history.empty())
				{
					status = "Nothing to undo";
					continue;
				}

				std::pair<int, int> last = move_history.back();
				move_history.pop_back();
				board[last.first][last.second] = ' ';
				current_player = (current_player == 'X') ? 'O' : 'X';
				status = "Last move undone";
				continue;
			}

			if (ch == KEY_MOUSE)
			{
				MEVENT event;
				if (getmouse(&event) == OK)
				{
					int row = 0;
					int col = 0;
					if ((event.bstate & BUTTON1_CLICKED) &&
							MouseToCell(event.y, event.x, board_size, origin_y, origin_x, &row, &col))
					{
						cursor_row = row;
						cursor_col = col;
						ch = '\n';
					}
				}
			}

			if (ch == '\n' || ch == KEY_ENTER || ch == 10 || ch == 13 || ch == ' ')
			{
				if (board[cursor_row][cursor_col] != ' ')
				{
					status = "Cell already occupied";
					continue;
				}

				board[cursor_row][cursor_col] = current_player;
				move_history.push_back({cursor_row, cursor_col});

				char detected_winner = ' ';
				std::vector<std::pair<int, int>> win_cells = GetWinningCells(board, &detected_winner);
				if (detected_winner != ' ')
				{
					winner = detected_winner;
					round_over = true;
					for (const std::pair<int, int> &cell : win_cells)
					{
						highlighted.insert(cell);
					}

					if (winner == 'X')
					{
						++score->x_wins;
					}
					else
					{
						++score->o_wins;
					}

					status = std::string("Player ") + winner + " wins! Press n for new round.";
					continue;
				}

				if (IsBoardFull(board))
				{
					round_over = true;
					++score->draws;
					status = "Draw game! Press n for new round.";
					continue;
				}

				current_player = (current_player == 'X') ? 'O' : 'X';
				status = "Move accepted";
			}
		}
	}

} // namespace

int main()
{
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	curs_set(0);
	InitColors();

	mousemask(BUTTON1_CLICKED, nullptr);

	if (LINES < 16 || COLS < 42)
	{
		endwin();
		printf("Terminal is too small. Please resize and try again.\n");
		return 1;
	}

	Score score;
	while (true)
	{
		int board_size = PromptBoardSize();
		if (board_size == 0)
		{
			break;
		}

		while (true)
		{
			RoundAction action = RunSingleGame(board_size, &score);
			if (action == RoundAction::kQuit)
			{
				endwin();
				return 0;
			}
			if (action == RoundAction::kChangeBoardSize)
			{
				break;
			}
		}
	}

	endwin();
	return 0;
}

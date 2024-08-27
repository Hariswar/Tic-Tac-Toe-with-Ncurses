#include <curses.h>

#include <iostream>
#include <string>

using namespace std;

char getCellSymbol(const string& board, int cell);

char getSymbol(const string& board, int x, int y, int size);

void printWelcomeMessage();

void displayMenu();

string initializeBoard(int size);

void printBoard(const string& board, int size);

void updateBoard(string& board, int chosenCell, char symbol);

bool checkWin(const string& board, int size, char symbol);

bool isFull(const string& board, int size);

void printGoodbyeMessage();

int main() {
	printWelcomeMessage();

	char cont;

	do {
		int option, size;
		do {
			displayMenu();

			cin >> option;
			switch (option) {
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
					size = option + 2;
					break;
				case 8:
					printGoodbyeMessage();
					return 0;
				default:
					cout << "Invalid option!" << endl;
					break;
			}
		} while (option < 1 || option > 9);

		int ch = '\0';
		MEVENT event;

		setlocale(LC_ALL, "");
		initscr();
		if (!has_colors()) {
			endwin();
			cout << "Your terminal does not support colors." << endl;
			return 1;
		}
		start_color();

		noecho();
		cbreak();
		keypad(stdscr, true);
		mousemask(BUTTON1_CLICKED | BUTTON2_CLICKED | BUTTON3_CLICKED, nullptr);
		init_pair(1, COLOR_WHITE, COLOR_GREEN);
		init_pair(2, COLOR_BLACK, COLOR_RED);

		string board = initializeBoard(size), error;
		bool done = false;
		int winner = -1, player = 1;
		int cell = 0;

		do {
			error = "";
			char symbol = player == 1 ? 'S' : 'T';

			switch (ch) {
				case KEY_MOUSE:
					if (getmouse(&event) == OK) {
						if (event.bstate & BUTTON1_CLICKED && !done) {
							// Because there's space between the cells, include that space in the bounds check
							// but disallow clicking on the spaces by checking for mod
							if (event.x < size * 3 && (event.x % 3 != 2) && event.y < size) {
								// 1-indexed so +1
								int cell = event.y * size + event.x / 3 + 1;

								// above is bounds check, no need to duplicate
								if (getCellSymbol(board, cell) == 'S' || getCellSymbol(board, cell) == 'T') {
									error = "Cell is already occupied; please enter another one.";
									cell = 0;
								} else {
									updateBoard(board, cell, symbol);

									cell = 0;
									if (checkWin(board, size, symbol)) {
										winner = player;
									} else {
										player = player == 1 ? 2 : 1;
									}

									done = isFull(board, size);
								}
							}
						}
					}
					break;
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					cell *= 10;
					cell += (ch - '0');
					break;
				case '\n':
					if (cell < 1 || cell > size * size) {
						error = "Invalid cell; enter a number between 1 and " + to_string(size * size) + ".";
						cell = 0;
					} else if (getCellSymbol(board, cell) == 'S' || getCellSymbol(board, cell) == 'T') {
						error = "Cell is already occupied; please enter another one.";
						cell = 0;
					} else {
						updateBoard(board, cell, symbol);

						cell = 0;
						if (checkWin(board, size, symbol)) {
							winner = player;
						} else {
							player = player == 1 ? 2 : 1;
						}

						done = isFull(board, size);
					}
					break;
				case '\0':
					break;
				default:
					goto playSkip;
			}

			clear();
			printBoard(board, size);

			if (!error.empty()) {
				attron(COLOR_PAIR(2));
				printw(error.c_str());
				attroff(COLOR_PAIR(2));
			}

			printw(("\nPlayer " + to_string(player) + ", select a cell: " + to_string(cell)).c_str());
		playSkip:
			void(0);
		} while (winner == -1 && !done && (ch = getch()) != 'q');

		endwin();

		if (ch != 'q') {
			if (winner == -1) {
				cout << "It's a tie!" << endl;
			} else {
				cout << "Player " << winner << " wins!" << endl;
			}
		}

		cout << "Would you like to continue?" << endl;
		cin >> cont;

		while (cont != 'y' && cont != 'Y' && cont != 'n' && cont != 'N') {
			cout << "Would you like to continue? (y/n)" << endl;
			cin >> cont;
		}
	} while (cont != 'n' && cont != 'N');

	printGoodbyeMessage();

	return 0;
}

char getCellSymbol(const string& board, int cell) {
	return board[(cell - 1) * 2];
}

char getSymbol(const string& board, int x, int y, int size) {
	return board[(y * size + x) * 2];
}

void printWelcomeMessage() {
	cout << "Welcome to Dynamic Tic-Tac-Toe Missouri S&T Edition!" << endl;
}

void displayMenu() {
	cout << "Choose the size of the Tic-Tac-Toe board or Quit:\n"
		 << "1. 3x3 Board\n"
		 << "2. 4x4 Board\n"
		 << "3. 5x5 Board\n"
		 << "4. 6x6 Board\n"
		 << "5. 7x7 Board\n"
		 << "6. 8x8 Board\n"
		 << "7. 9x9 Board\n"
		 << "8. Quit the Program" << endl;
}

string initializeBoard(int size) {
	string out;

	for (int i = 1; i <= size * size; i++) {
		if (i < 10) {
			out += '0' + to_string(i);
		} else {
			out += to_string(i);
		}
	}

	return out;
}

void printBoard(const string& board, int size) {
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			printw(board.substr((i * size + j) * 2, 2).c_str());
			addch(' ');
		}

		addch('\n');
	}
}

void updateBoard(string& board, int chosenCell, char symbol) {
	board[(chosenCell - 1) * 2] = symbol;
	board[(chosenCell - 1) * 2 + 1] = symbol;
}

bool checkWin(const string& board, int size, char symbol) {
	// rows & cols
	for (int i = 0; i < size; i++) {
		if (getSymbol(board, i, 0, size) == symbol) {
			bool full = true;

			for (int j = 1; j < size; j++) {
				if (getSymbol(board, i, j, size) != symbol) {
					full = false;
					break;
				}
			}

			if (full) {
				return true;
			}
		}

		if (getSymbol(board, 0, i, size) == symbol) {
			bool full = true;

			for (int j = 1; j < size; j++) {
				if (getSymbol(board, j, i, size) != symbol) {
					full = false;
					break;
				}
			}

			if (full) {
				return true;
			}
		}
	}

	bool leftDiagonal = true, rightDiagonal = true;
	for (int i = 0; i < size; i++) {
		if (getSymbol(board, i, i, size) != symbol) {
			leftDiagonal = false;
		}

		if (getSymbol(board, i, size - i - 1, size) != symbol) {
			rightDiagonal = false;
		}
	}

	return leftDiagonal || rightDiagonal;
}

bool isFull(const string& board, int size) {
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			char symbol = getSymbol(board, i, j, size);

			if (symbol != 'S' && symbol != 'T') {
				return false;
			}
		}
	}

	return true;
}

void printGoodbyeMessage() {
	cout << "Goodbye!" << endl;
}
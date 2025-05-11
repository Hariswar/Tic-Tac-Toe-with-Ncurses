# Tic Tac Toe with Ncurses
Designed and implemented a dynamic Tic-Tac-Toe game in C++ using the Ncurses library for terminal-based graphical representation, with board sizes ranging from 3x3 to 9x9. I used the feature called Ncurses which integrating ncurses for the main gameplay loop and utilizing the mouse interactions feature to allow users to click on the square to play on it. It provides an interactive experience where players can choose board dimensions and play the game in the terminal.

## Features
- Dynamic board sizes ranging from 3x3 to 9x9.
- Intuitive graphical interface built using the Ncurses library.
- Support for mouse input for cell selection.
- Turn-based gameplay for two players.
- Detection of win, loss, or draw scenarios.
- Graceful exit functionality.

## Requirements
- A terminal with Ncurses support.
- C++ compiler (e.g., g++).
- Ncurses library installed on the system.

## Compilation Instructions
1. Ensure you have Ncurses installed on your system.
2. Compile the program using g++:
   ```
   g++ -o tic_tac_toe.cpp -lncurses
   ```

## Usage Instructions
1. Run the compiled program:
   ```
   ./tic_tac_toe
   ```
2. Follow the on-screen menu to choose the board size or quit the program.
3. Use your mouse or keyboard to select cells during your turn.
4. The game ends when a player wins or the board is full.

## Gameplay Instructions
- **Start the Game:** After choosing the board size, the game initializes with an empty board.
- **Turn Indicators:** Each player takes turns. Player 1 is represented by `S`, and Player 2 is represented by `T`.
- **Winning Conditions:**
  - Complete a row, column, or diagonal with your symbol.
- **Drawing:** If the board is filled without a winner, the game ends in a draw.

## Limitations
- Requires a terminal that supports Ncurses.
- Limited to two players.

## License
This project is licensed under the MIT License. See `LICENSE` for details.

## Acknowledgments
- Ncurses documentation and tutorials.
- Inspiration from classic Tic Tac Toe implementations.


#include <chrono>
#include <iostream>

#include "engine.hpp"

int main() {
  Engine engine;
  Board board;

  // Test FEN
  //   engine.setPosition(
  //       "r1bq1rk1/pp1n1ppp/2p1pn2/8/2BP4/2N2N2/PPP2PPP/R1BQK2R w KQ - 0 10");

  // Mate in 3
  //   engine.setPosition("rnqk4/pppppr2/6r1/8/8/7Q/PPPPPPPP/RNB1K1NR w KQq - 0
  //   1");

  //   // mate in 1
  //   engine.setPosition("rnqk4/ppppp3/8/8/8/7Q/PPPPPPPP/RNB1K1NR w KQq - 0
  //   1");

  //   engine.setPosition(
  //       "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

  //   engine.printBoard();

  bool gameOver = false;
  while (!gameOver) {
    // Measure time taken for depth 4 search
    auto start = std::chrono::high_resolution_clock::now();
    std::string bestMove = engine.getBestMove(4);
    if (bestMove == "null") {
      std::cout << "no move found";
      break;
    }
    auto end = std::chrono::high_resolution_clock::now();

    Movelist moves;
    movegen::legalmoves(moves, board);

    for (const auto& move : moves) {
      if (uci::moveToUci(move) == bestMove) {
        board.makeMove(move);
        engine.setPosition(board.getFen());
        engine.printBoard();
      }
    }

    std::chrono::duration<double> duration = end - start;

    std::cout << "Best move: " << bestMove << "\n";
    std::cout << "Time taken: " << duration.count() << " seconds\n";
  }

  return 0;
}

#include <iostream>

#include "./engine/engine.hpp"

int main() {
  Engine engine;
  engine.initializeEngine();

  // Example position (FEN for starting position)
  std::string fen =
      "rnbqkb1r/pppp1ppp/4pn2/8/2B1P3/8/PPPP1PPP/RNBQK1NR w KQkq - 2 3";
  engine.setPosition(fen);

  std::cout << "Current Position:\n";
  engine.printBoard();

  int searchDepth = 4;
  std::string bestMove = engine.getBestMove(searchDepth);

  std::cout << "Best Move: " << bestMove << std::endl;

  return 0;
}

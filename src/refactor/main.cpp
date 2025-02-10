#include "engine.hpp"

int main() {
  Engine engine;
  engine.initializeEngine();
  engine.setPosition(
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  engine.printBoard();
  std::cout << engine.getBestMove(4) << "\n";
  return 0;
}
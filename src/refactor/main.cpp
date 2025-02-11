#include "engine.hpp"

int main() {
  Engine engine;
  engine.initializeEngine();
  engine.setPosition("3k4/8/8/3K2R1/7R/8/8/8 w - - 0 1");
  engine.printBoard();
  std::cout << engine.getBestMove(4) << "\n";
  return 0;
}
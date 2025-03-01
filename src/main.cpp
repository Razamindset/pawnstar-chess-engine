#include <iostream>

#include "./engine/engine.hpp"

int main() {
  Engine engine;
  engine.printBoard();

  std::cout << engine.getBestMove(5) << "\n";
  return 0;
}

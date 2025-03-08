#include <iostream>

#include "./engine/engine.hpp"

int main() {
  Engine engine;
  // ! i have done this but there is still some issues the engine is not able to
  // find mates or like push the king to the edge of the board

  // Todo Implement uci comp for better testing and add history heuristics
  engine.setPosition("K7/8/1qk5/8/8/8/8/8 b - - 13 14");
  engine.printBoard();

  std::cout << engine.getBestMove(4) << "\n";
  std::cout << engine.getTableSize() << "\n";
  std::cout << engine.getTableMemoryUsage() << " kBs" << "\n";
  return 0;
}

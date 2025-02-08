
#include "engine.hpp"

int main() {
  Engine engine;
  // mate in two for white
  engine.setPostion("rnqk4/pppppr2/8/8/8/7Q/PPPPPPPP/RNB1K1NR w KQq - 0 1");

  // Mate in one for black
  engine.setPostion("8/8/8/8/8/8/5qk1/7K b KQq - 0 1");

  engine.printBoard();
  std::cout << engine.getBestMove(4);
}
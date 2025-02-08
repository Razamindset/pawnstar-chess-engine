
#include "engine.hpp"

int main() {
  Engine engine;
  engine.setPostion("rnqk4/pppppr2/8/8/8/7Q/PPPPPPPP/RNB1K1NR w KQq - 0 1");
  engine.printBoard();
  std::cout << engine.getBestMove(4);
}
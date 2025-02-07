#include <iostream>

#include "engine.hpp"

int main() {
  Engine engine;
  engine.initializeEngine();
  // Mate postion
  // engine.setPostion("4kbn1/p2pppr1/B6p/8/8/4PN2/P2P1PpP/2R1K1R1 w - - 4 8");

  // mate in 2
  engine.setPostion("rnqk4/pppppr2/8/8/8/7Q/PPPPPPPP/RNB1K1NR w KQq - 0 1");

  // Mate in 3
  engine.setPostion("rnqk4/pppppr2/6r1/8/8/7Q/PPPPPPPP/RNB1K1NR w KQq - 0 1");

  // engine.setPostion("2r1kbnr/p2ppp1p/B7/8/8/4PN2/P2P1PpP/R3K1R1 w k - 2 5");
  std::cout << engine.getBestMove(6);
  return 0;
}
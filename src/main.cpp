#include <iostream>

#include "engine.hpp"

int main() {
  Engine engine;
  engine.initializeEngine();
  engine.setPostion("2r1kbnr/p2ppp1p/B7/8/8/4PN2/P2P1PpP/R3K1R1 w k - 2 5");
  std::cout << engine.getBestMove(6);
  return 0;
}
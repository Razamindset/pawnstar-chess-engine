#include "engine.hpp"

int main() {
  Engine engine;
  engine.initializeEngine();
  engine.setPosition(
      "r3k2r/p1ppqpb1/Bn2pnp1/3PN3/4P3/2B5/PPP2PQP/R3K2R b KQkq - 0 3");
  engine.printBoard();
  std::cout << engine.getBestMove(4) << "\n";
  return 0;
}
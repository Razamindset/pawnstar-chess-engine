#include <chrono>
#include <iostream>

#include "engine.hpp"

int main() {
  Engine engine;
  engine.initializeEngine();

  // Test FEN
  engine.setPosition(
      "r1bq1rk1/pp1n1ppp/2p1pn2/8/2BP4/2N2N2/PPP2PPP/R1BQK2R w KQ - 0 10");

  // Middle game test pos
  engine.setPosition(
      "r3k2r/ppp2ppp/2n1b3/2B1P3/4P3/2P5/PP3PPP/RNB2RK1 b kq - 0 1");

  engine.printBoard();

  // Measure time taken for depth 4 search
  auto start = std::chrono::high_resolution_clock::now();
  std::string bestMove = engine.getBestMove(4);
  auto end = std::chrono::high_resolution_clock::now();

  std::chrono::duration<double> duration = end - start;

  std::cout << "Best move: " << bestMove << "\n";
  std::cout << "Time taken: " << duration.count() << " seconds\n";

  return 0;
}

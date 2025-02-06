#include <iostream>

#include "engine.hpp"

int main() {
  Engine engine;
  engine.initializeEngine();
  engine.setPostion("5br1/P3kp2/4p3/8/7p/4PN2/3P1PpP/4K1R1 w - - 0 13");
  std::cout << engine.getBestMove(6);
  return 0;
}
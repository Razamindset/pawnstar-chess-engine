#include <iostream>

#include "engine.hpp"

int main() {
  Engine engine;
  engine.initializeEngine();
  std::cout << engine.getBestMove(5);
  return 0;
}
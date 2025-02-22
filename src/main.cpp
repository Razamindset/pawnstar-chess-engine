#include <iostream>

#include "./game-recorder/pgn-recorder.hpp"

int main() {
  Engine engine;
  std::string fen;

  while (true) {
    std::cout << "Enter FEN (or type 'exit' to quit): ";
    std::getline(std::cin, fen);

    if (fen == "exit") break;

    engine.setPosition(fen);
    std::string bestMove = engine.getBestMove(6);

    std::cout << "Best Move: " << bestMove << "\n";
  }

  return 0;
}

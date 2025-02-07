#include <chess.hpp>
#include <iostream>

#include "engine.hpp"

using namespace chess;

int main() {
  Engine engine;
  engine.initializeEngine();
  // Mate postion
  // engine.setPostion("4kbn1/p2pppr1/B6p/8/8/4PN2/P2P1PpP/2R1K1R1 w - - 4 8");

  // mate in 2
  engine.setPostion("rnqk4/pppppr2/8/8/8/7Q/PPPPPPPP/RNB1K1NR w KQq - 0 1");

  // Mate in 3
  engine.setPostion("rnqk4/pppppr2/6r1/8/8/7Q/PPPPPPPP/RNB1K1NR w KQq - 0 1");

  // Starting postion
  engine.setPostion("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

  // engine.setPostion("2r1kbnr/p2ppp1p/B7/8/8/4PN2/P2P1PpP/R3K1R1 w k - 2 5");
  // std::cout << engine.getBestMove(6);
  bool humanTurn = true;
  Board board;
  while (!engine.isGameOver()) {
    if (!humanTurn) {
      engine.setPostion(board.getFen());
      auto bestMove = engine.getBestMove(4);
      std::cout << "Engine best move: " << bestMove << "\n";
    }
    Movelist moves;
    movegen::legalmoves(moves, board);

    std::cout << board << "\n";

    for (const auto& move : moves) {
      std::cout << move << " ";
    }
    std::cout << "\n";

    std::string inMove;
    std::cin >> inMove;

    for (const auto& move : moves) {
      if (uci::moveToUci(move) == inMove) {
        board.makeMove(move);
        humanTurn = !humanTurn;

        break;
      }
    }
  }

  return 0;
}
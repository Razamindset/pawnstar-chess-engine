#include <iostream>
#include <random>
#include <vector>

#include "chess.hpp"
#include "engine.hpp"

using namespace chess;

int main() {
  Engine engine;
  Board board;

  std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
  engine.initializeEngine();
  engine.setPosition(fen);
  board.setFen(fen);

  bool isGameOver = false;

  while (!isGameOver) {
    engine.printBoard();
    Movelist moves;
    movegen::legalmoves(moves, board);

    if (moves.empty()) {
      std::cout << "Game Over!" << std::endl;
      break;
    }

    // 1. White (Random move)
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> dist(0, moves.size() - 1);
    Move randomMove = moves[dist(rng)];

    board.makeMove(randomMove);
    engine.setPosition(board.getFen());

    std::cout << "White plays: " << uci::moveToUci(randomMove) << std::endl;

    if (engine.isGameOver(board)) {
      std::cout << "White wins!" << std::endl;
      GameResultReason reason = engine.getGameOverReason(board);
      if (reason == GameResultReason::CHECKMATE) {
        std::cout << "Ckeckmate \n";
      } else {
        std::cout << "Draq \n";
      }
      engine.printBoard();
      break;
    }

    // 2. Black (Engine Move)
    engine.printBoard();
    moves.clear();
    movegen::legalmoves(moves, board);

    if (moves.empty()) {
      std::cout << "Game Over!" << std::endl;
      break;
    }

    std::string bestMove = engine.getBestMove(4);
    bool moveMade = false;

    for (const auto& move : moves) {
      if (uci::moveToUci(move) == bestMove) {
        board.makeMove(move);
        engine.setPosition(board.getFen());
        moveMade = true;
        break;
      }
    }

    if (!moveMade) {
      std::cerr << "Error: No valid move found for engine!" << std::endl;
      break;
    }

    std::cout << "Black (Engine) plays: " << bestMove << std::endl;

    if (engine.isGameOver(board)) {
      std::cout << "Black (Engine) wins!" << std::endl;
      engine.printBoard();
      break;
    }
  }

  return 0;
}

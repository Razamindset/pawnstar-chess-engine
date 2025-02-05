#include "engine.hpp"

void Engine::initializeEngine() { std::cout << "Enigne Initialized!\n"; }

void Engine::setPostion(const std::string fen) { board = Board(fen); }

bool Engine::isGameOver() {
  std::pair<chess::GameResultReason, chess::GameResult> result =
      board.isGameOver();

  if (result.second != GameResult::NONE) {
    return true;
  }
  return false;
}

std::string Engine::getBestMove(int depth) {
  if (isGameOver()) return "null";
  Movelist moves;
  movegen::legalmoves(moves, board);

  if (!moves.empty()) {
    return uci::moveToUci(moves[0]);
  }

  return "null";
}
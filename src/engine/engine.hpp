#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <string>

#include "../chess-library/include/chess.hpp"

using namespace chess;

class Engine {
 private:
  Board board;
  int getPieceValue(Piece piece);

 public:
  int positionsSearched = 0;

  void setPosition(const std::string& fen);
  void printBoard();
  void initilizeEngine();

  std::string getBestMove(int depth);

  // Move making
  void makeMove(std::string move);

  bool isGameOver() {
    auto result = board.isGameOver();
    return result.second != GameResult::NONE;
  }

  GameResultReason getGameOverReason() {
    auto result = board.isGameOver();
    return result.first;
  }

  std::string moveToSan(const std::string& uciMove);
};

#endif
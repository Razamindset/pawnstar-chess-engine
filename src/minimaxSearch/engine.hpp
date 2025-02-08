#ifndef ENGINE_H
#define ENGINE_H

#include <algorithm>
#include <string>

#include "chess.hpp"

using namespace chess;

constexpr int MATE_SCORE = 100000;

class Engine {
 private:
  Board board;
  int positionsSearched = 0;
  int evaluatePosition(const Board& board);
  int minmaxSearch(int depth, int alpha, int beta, bool maximizingPlayer);

 public:
  bool isGameOver(const Board& board);
  void initializeEngine();
  void setPostion(const std::string& fen);
  void printBoard();
  Move getBestMove(int depth);
  GameResultReason getGameOverReason(const Board& board);
};
#endif
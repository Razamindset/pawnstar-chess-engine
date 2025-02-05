#ifndef ENGINE_H
#define ENGINE_H

#include <algorithm>
#include <string>

#include "chess.hpp"

using namespace chess;

class Engine {
 private:
  Board board;
  int evaluatePosition(const Board &board);
  const int pieceValues[6] = {100, 320, 330, 500, 900, 20000};
  int minmax(int depth, bool maximizingPlayer, int alpha, int beta);

 public:
  bool isGameOver();
  void initializeEngine();
  void setPostion(const std::string fen);
  std::string getBestMove(int depth);
};

#endif
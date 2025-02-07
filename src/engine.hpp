#ifndef ENGINE_H
#define ENGINE_H

#include <algorithm>
#include <limits>
#include <string>

#include "chess.hpp"

using namespace chess;

constexpr int MATE_SCORE = 1000000;

class Engine {
 private:
  Board board;
  int evaluatePosition(const Board& board);
  const int pieceValues[6] = {100, 320, 330, 500, 900, 20000};
  int search(int depth, int aplha, int beta);
  void orderMoves(Movelist& moves);
  int getPieceValue(Piece piece);
  int searchAllCaptures(int alpha, int beta);
  chess::PieceType getPieceType(PieceGenType piece);
  int positionsSearched;

 public:
  bool isGameOver();
  void initializeEngine();
  void setPostion(const std::string fen);
  std::string getBestMove(int depth);
};

#endif
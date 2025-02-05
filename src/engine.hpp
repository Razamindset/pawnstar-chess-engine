#ifndef ENGINE_H
#define ENGINE_H

#include <string>

#include "chess.hpp"

using namespace chess;

class Engine {
 private:
  Board board;

 public:
  void initializeEngine();
  void setPostion(const std::string fen);
  bool isGameOver();
  std::string getBestMove(int depth);
};

#endif
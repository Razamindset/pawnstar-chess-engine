#ifndef EVAL_HPP
#define EVAL_HPP

#include "../chess-library/include/chess.hpp"
#include "utils.hpp"

using namespace chess;

class Evaluator {
 private:
  Board& board;

 public:
  Evaluator(Board& b) : board(b) {}
  int evaluatePosition(const Board& board);
};

#endif
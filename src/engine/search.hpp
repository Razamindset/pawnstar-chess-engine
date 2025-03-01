#ifndef SEARCH_HPP
#define SEARCH_HPP

#include "../chess-library/include/chess.hpp"
#include "eval.hpp"
#include "utils.hpp"

using namespace chess;

class Search {
 private:
  Board& board;
  Evaluator evaluator;

  int positionsSearched;

  int negaMax(int depth, int alpha, int beta);
  void orderMoves(Movelist& moves, bool isWinning, Board& board);

 public:
  Search(Board& b) : board(b), evaluator(b) {}

  std::string searchBestMove(int depth);

  int getSearchedPos() { return positionsSearched; }
};

#endif
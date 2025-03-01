#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <string>

#include "../chess-library/include/chess.hpp"
#include "search.hpp"
#include "utils.hpp"

using namespace chess;

class Engine {
 private:
  Board board;
  Search search;

 public:
  Engine() : search(board) {}
  void setPosition(const std::string& fen);
  void printBoard();
  std::string getBestMove(int depth) { return search.searchBestMove(depth); };
};

#endif
#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <string>

#include "../chess-library/include/chess.hpp"
#include "piece-maps.hpp"
#include "utils.hpp"

using namespace chess;

class Engine {
 private:
  Board board;

  // Pieces related
  int getPieceValue(Piece piece);

  // Search related
  int negaMax(int depth, int alpha, int beta);
  void orderMoves(Movelist& moves);

  // Evaluation related fuctions
  int evaluatePosition(const Board& board);
  int evaluateMaterial(const Board& board);
  int evaluatePieceSquareTables(const Board& board, bool isEndGame);
  int evaluatePawnStructure(const Board& board);  // Todo
  int evaluateRookFiles(const Board& board);      // Todo
  int evaluateMobility(const Board& board);       // Todo did something

  // Engame Specific evalution stuff
  int kingEndgameScore(const Board& board, Color us, Color op);
  int manhattanDistance(Square sq1, Square sq2) {
    int file1 = sq1.file();
    int rank1 = sq1.rank();
    int file2 = sq2.file();
    int rank2 = sq2.rank();

    return abs(file1 - file2) + abs(rank1 - rank2);
  };

 public:
  void setPosition(const std::string& fen);
  void printBoard();
  std::string getBestMove(int depth);

  int positionsSearched = 0;

  bool isGameOver() {
    auto result = board.isGameOver();
    return result.second != GameResult::NONE;
  }

  GameResultReason getGameOverReason() {
    auto result = board.isGameOver();
    return result.first;
  }
};

#endif
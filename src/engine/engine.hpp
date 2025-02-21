#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <string>

#include "../chess-library/include/chess.hpp"
#include "piece_values.hpp"

using namespace chess;

constexpr int mirrorIndex(int sq) {
  return sq ^ 56;  // Flips the rank (0-7 ↔ 7-0) while keeping the file the same
}

constexpr int MARGIN = 50;

//! All the code is written by myself. I have taken advise from coding
// assistants. But i have tried to write all of it myself. I may ask chatgpt for
// correcting grammer mistakes in the comments I write so some of those may feel
// like ai generated. For the most part i have written comments without help
// from ai so that i can explain my own thought process and understanding. I
// openly ask for suggestions on any errors found and may be reported, will be
// apreciated.

// Chatgpt suggests that a 24 bit hash is too small as 16 million possible
// unique positions (2²⁴ = 16,777,216) can be represented. I think this may have
// affected our last tt implementation.
// The soultion is that we store the hash and a ply(move number) along with it
// to make it unique

struct HistoryEntry {
  uint32_t hash;  // 24-bit or 32-bit board hash
  uint8_t ply;    // Move number (half-move count)
};

class Engine {
 private:
  Board board;
  int positionsSearched = 0;
  std::vector<HistoryEntry> positionHistory;
  int negaMax(int depth, int alpha, int beta);
  int evaluatePosition(const Board& board);
  int searchAllCaptures(int alpha, int beta);
  int getPieceValue(Piece piece);
  int kingEndgameScore(const Board& board, Color us, Color op);
  int manhattanDistance(Square sq1, Square sq2) {
    int file1 = sq1.file();
    int rank1 = sq1.rank();
    int file2 = sq2.file();
    int rank2 = sq2.rank();

    return abs(file1 - file2) + abs(rank1 - rank2);
  }
  void orderMoves(Movelist& moves, bool isWinning);

 public:
  void initializeEngine();
  void setPosition(const std::string& fen);
  void opponentMoves(Move move);
  void printBoard();
  std::string getBestMove(int depth);
  GameResultReason getGameOverReason(const Board& board);
  bool isGameOver(const Board& board);
};

#endif
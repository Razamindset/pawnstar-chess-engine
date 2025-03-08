#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <string>

#include "../chess-library/include/chess.hpp"
#include "piece-maps.hpp"
#include "utils.hpp"

using namespace chess;

// Transposition table entry types
enum class TTEntryType {
  EXACT,  // Exact score for the position
  LOWER,  // Lower bound (alpha cutoff)
  UPPER   // Upper bound (beta cutoff)
};

// Structure for transposition table entries
struct TTEntry {
  uint64_t hash;     // Zobrist hash of the position
  int score;         // Evaluation score
  int depth;         // Depth at which the position was evaluated
  TTEntryType type;  // Type of entry
  Move bestMove;     // Best move found for this position
};

class Engine {
 private:
  Board board;

  // transpostion table realated
  std::unordered_map<uint64_t, TTEntry> transpositionTable;
  int ttHits = 0;
  void clearTranspositionTable();
  bool probeTT(uint64_t hash, int depth, int& score, int alpha, int beta,
               Move& bestMove);
  void storeTT(uint64_t hash, int depth, int score, TTEntryType type,
               Move bestMove);

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

  // Tts size
  size_t getTableSize() const { return transpositionTable.size(); }

  /* Get table size in Kilobytes */
  size_t getTableMemoryUsage() const {
    // Size of each entry + size of the key (array) * number of entries
    size_t size = (sizeof(TTEntry) + sizeof(std::array<unsigned char, 24>)) *
                  transpositionTable.size();
    return size / 1024;
  }

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
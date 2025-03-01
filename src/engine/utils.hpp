#ifndef UTILS_HPP
#define UTILS_HPP

#include "../chess-library/include/chess.hpp"

using namespace chess;

constexpr int MATE_SCORE = 100000;

constexpr int KNIGHT_MAP[64] = {
    -50, -40, -30, -30, -30, -30, -40, -50, -40, -20, 0,   5,   5,
    0,   -20, -40, -30, 5,   10,  15,  15,  10,  5,   -30, -30, 0,
    15,  20,  20,  15,  0,   -30, -30, 5,   15,  20,  20,  15,  5,
    -30, -30, 0,   10,  15,  15,  10,  0,   -30, -40, -20, 0,   0,
    0,   0,   -20, -40, -50, -40, -30, -30, -30, -30, -40, -50};
constexpr int PAWN_MAP[64] = {
    0,  0,  0,  0,   0,   0,  0,  0,   // Rank 8
    50, 50, 50, 50,  50,  50, 50, 50,  // Rank 7
    30, 30, 30, 40,  40,  30, 30, 30,  // Rank 6
    10, 10, 20, 30,  30,  20, 10, 10,  // Rank 5
    5,  5,  10, 25,  25,  10, 5,  5,   // Rank 4
    0,  0,  0,  20,  20,  0,  0,  0,   // Rank 3
    5,  10, 10, -20, -20, 10, 10, 5,   // Rank 2
    0,  0,  0,  0,   0,   0,  0,  0    // Rank 1
};
constexpr int QUEEN_MAP[64] = {
    -20, -10, -10, -5, -5, -10, -10, -20, -10, 0,   5,   0,  0,  5,   0,   -10,
    -10, 5,   5,   5,  5,  5,   5,   -10, -5,  0,   5,   5,  5,  5,   0,   -5,
    0,   0,   5,   5,  5,  5,   0,   -5,  -10, 5,   5,   5,  5,  5,   0,   -10,
    -10, 0,   5,   0,  0,  0,   0,   -10, -20, -10, -10, -5, -5, -10, -10, -20};
constexpr int KING_MAP[64] = {
    -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50,
    -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30, -30, -40,
    -40, -50, -50, -40, -40, -30, -20, -30, -30, -40, -40, -30, -30,
    -20, -10, -20, -20, -20, -20, -20, -20, -10, 20,  20,  0,   0,
    0,   0,   20,  20,  20,  30,  10,  0,   0,   10,  30,  20};
constexpr int BISHOP_MAP[64] = {
    -20, -10, -10, -10, -10, -10, -10, -20, -10, 0,   0,   0,   0,
    0,   0,   -10, -10, 0,   5,   10,  10,  5,   0,   -10, -10, 5,
    5,   10,  10,  5,   5,   -10, -10, 0,   10,  10,  10,  10,  0,
    -10, -10, 10,  10,  10,  10,  10,  10,  -10, -10, 5,   0,   0,
    0,   0,   5,   -10, -20, -10, -10, -10, -10, -10, -10, -20};
constexpr int ROOK_MAP[64] = {
    0,  0,  0,  0,  0,  0,  0,  0,   // Rank 8
    20, 20, 20, 20, 20, 20, 20, 20,  // Rank 7
    0,  0,  0,  0,  0,  0,  0,  0,   // Rank 6
    0,  0,  0,  0,  0,  0,  0,  0,   // Rank 5
    0,  0,  0,  0,  0,  0,  0,  0,   // Rank 4
    0,  0,  0,  0,  0,  0,  0,  0,   // Rank 3
    0,  0,  0,  0,  0,  0,  0,  0,   // Rank 2
    5,  10, 10, 10, 10, 10, 10, 5    // Rank 1
};
constexpr int KING_ENDGAME_MAP[64] = {
    -50, -30, -10, 0,  0,  -10, -30, -50, -30, -10, 20,  30, 30, 20,  -10, -30,
    -10, 20,  40,  50, 50, 40,  20,  -10, 0,   30,  50,  60, 60, 50,  30,  0,
    0,   30,  50,  60, 60, 50,  30,  0,   -10, 20,  40,  50, 50, 40,  20,  -10,
    -30, -10, 20,  30, 30, 20,  -10, -30, -50, -30, -10, 0,  0,  -10, -30, -50};

constexpr int MARGIN = 50;

constexpr int mirrorIndex(int sq) {
  return sq ^ 56;  // Flips the rank (0-7 ↔ 7-0) while keeping the file the same
}

// Utility functions for distance calculations
inline int manhattanDistance(Square sq1, Square sq2) {
  int file1 = sq1.file();
  int rank1 = sq1.rank();
  int file2 = sq2.file();
  int rank2 = sq2.rank();

  return abs(file1 - file2) + abs(rank1 - rank2);
}

int getPieceValue(Piece piece);

GameResultReason getGameOverReason(const Board& board);
bool isGameOver(const Board& board);

#endif
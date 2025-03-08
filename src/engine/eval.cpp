#include "engine.hpp"

int Engine::evaluateMaterial(const Board& board) {
  constexpr int PAWN_VALUE = 100;
  constexpr int KNIGHT_VALUE = 300;
  constexpr int BISHOP_VALUE = 320;
  constexpr int ROOK_VALUE = 500;
  constexpr int QUEEN_VALUE = 900;

  auto countMaterial = [&](Color color) {
    return board.pieces(PieceType::PAWN, color).count() * PAWN_VALUE +
           board.pieces(PieceType::KNIGHT, color).count() * KNIGHT_VALUE +
           board.pieces(PieceType::BISHOP, color).count() * BISHOP_VALUE +
           board.pieces(PieceType::ROOK, color).count() * ROOK_VALUE +
           board.pieces(PieceType::QUEEN, color).count() * QUEEN_VALUE;
  };

  return countMaterial(Color::WHITE) - countMaterial(Color::BLACK);
}

int Engine::evaluatePieceSquareTables(const Board& board, bool isEndGame) {
  int eval = 0;

  for (Square sq = 0; sq < 64; sq++) {
    Piece piece = board.at(sq);
    if (piece.type() == PieceType::NONE) continue;

    int index =
        (piece.color() == Color::WHITE) ? sq.index() : mirrorIndex(sq.index());
    int squareValue = 0;

    switch (piece.type()) {
      case PAWN:
        squareValue = PAWN_TABLE[index];
        break;
      case KNIGHT:
        squareValue = KNIGHT_TABLE[index];
        break;
      case BISHOP:
        squareValue = BISHOP_TABLE[index];
        break;
      case ROOK:
        squareValue = ROOK_TABLE[index];
        break;
      case QUEEN:
        squareValue = QUEEN_TABLE[index];
        break;
      case KING:
        squareValue =
            isEndGame ? KING_END_TABLE[index] : KING_MIDDLE_TABLE[index];
        break;
    }

    eval += (piece.color() == Color::WHITE) ? squareValue : -squareValue;
  }

  return eval;
}

int Engine::evaluatePawnStructure(const Board& board) {
  int eval = 0;
  return eval;
}

int Engine::evaluateRookFiles(const Board& board) {
  int eval = 0;
  return eval;
}

/* The side which has more choices is generally better */
int Engine::evaluateMobility(const Board& board) {
  int eval = 0;
  Color us = board.sideToMove();
  Color op = us == Color::WHITE ? Color::BLACK : Color::WHITE;

  // Create temporary board to generate moves
  Board tempBoard = board;

  Movelist ourMoves;
  movegen::legalmoves(ourMoves, tempBoard);
  eval += ourMoves.size() * 5;  // 5 points per legal move

  tempBoard.makeNullMove();
  Movelist theirMoves;
  movegen::legalmoves(theirMoves, tempBoard);
  eval -= theirMoves.size() * 5;
  tempBoard.unmakeNullMove();

  return eval;
}

int Engine::kingEndgameScore(const Board& board, Color us, Color op) {
  Square ourKing = board.kingSq(us);
  Square opponentKing = board.kingSq(op);

  int score = 0;

  int opKingFile = opponentKing.file();
  int opKingRank = opponentKing.rank();

  // Higher bonus for corner squares
  if ((opKingFile == 0 || opKingFile == 7) &&
      (opKingRank == 0 || opKingRank == 7)) {
    score += 50;
  }
  // Bonus for being on the edge
  else if (opKingFile == 0 || opKingFile == 7 || opKingRank == 0 ||
           opKingRank == 7) {
    score += 30;
  }

  int distance = manhattanDistance(ourKing, opponentKing);
  score += (14 - distance) * 6;  // Closer = higher score

  return score;
}

int Engine::evaluatePosition(const Board& board) {
  if (isGameOver()) {
    if (getGameOverReason() == GameResultReason::CHECKMATE) {
      return (board.sideToMove() == Color::WHITE) ? MATE_SCORE : -MATE_SCORE;
    }
    return 0;
  }

  int eval = 0;

  bool isEndgame = (board.pieces(PieceType::QUEEN, Color::WHITE).count() +
                        board.pieces(PieceType::QUEEN, Color::BLACK).count() ==
                    0);

  eval += evaluateMaterial(board);
  eval += evaluatePieceSquareTables(board, isEndgame);
  eval += evaluatePawnStructure(board);
  eval += evaluateRookFiles(board);
  eval += evaluateMobility(board);

  //* If it is an endgame then we want the opponent king on specific squares
  if (isEndgame) {
    eval += kingEndgameScore(board, Color::WHITE, Color::BLACK) -
            kingEndgameScore(board, Color::BLACK, Color::WHITE);
  }

  return (board.sideToMove() == Color::WHITE) ? eval : -eval;
}

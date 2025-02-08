#include "engine.hpp"

void Engine::initializeEngine() { std::cout << "Engine Initialized \n"; }

void Engine::setPostion(const std::string& fen) { board = Board(fen); }

void Engine::printBoard() { std::cout << board; }

bool Engine::isGameOver(const Board& board) {
  auto result = board.isGameOver();
  return result.second != GameResult::NONE;
}

GameResultReason Engine::getGameOverReason(const Board& board) {
  auto result = board.isGameOver();
  return result.first;
}

int Engine::evaluatePosition(const Board& board) {
  // First check if gameOver
  if (isGameOver(board)) {
    if (getGameOverReason(board) == GameResultReason::CHECKMATE) {
      return (board.sideToMove() == Color::WHITE) ? -MATE_SCORE : MATE_SCORE;
    }
    return 0;
  }

  // Count material
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

  int eval = countMaterial(Color::WHITE) - countMaterial(Color::BLACK);

  return eval;
}

int Engine::minmaxSearch(int depth, bool maximizingPlayer) {
  positionsSearched++;
  if (depth == 0 || isGameOver(board)) {
    return evaluatePosition(board);
  }
  Movelist moves;
  movegen::legalmoves(moves, board);

  // Return evaluation if no legal moves
  if (moves.empty()) {
    return evaluatePosition(board);
  }

  if (maximizingPlayer) {
    // Maximinzing from -ive extreme
    int bestEval = -MATE_SCORE;

    for (const auto& move : moves) {
      board.makeMove(move);
      int evaluation = minmaxSearch(depth - 1, false);
      board.unmakeMove(move);

      bestEval = std::max(evaluation, bestEval);
    }

    return bestEval;

  } else {
    // Minimizing from +ive extreme
    int bestEval = MATE_SCORE;

    for (const auto& move : moves) {
      board.makeMove(move);
      int evaluation = minmaxSearch(depth - 1, true);
      board.unmakeMove(move);

      bestEval = std::min(evaluation, bestEval);
    }

    return bestEval;
  }
}

Move Engine::getBestMove(int depth) {
  Movelist moves;
  chess::movegen::legalmoves(moves, board);
  positionsSearched = 0;

  if (!moves.empty()) {
    // For min max search we have a maximizing player and a minimizing player
    // Lets say it is white to move and the best Evaluation is set to -infinity
    // Now white is the maximizing player
    // White is looking for any move that can give better evalution than
    // negative infinty/maximum
    // With each search we can update the best eval and the best move
    // We will return the best move at the end

    Move bestMove = moves[0];

    // The worst outcome can only result in a mate as we donot have infinty here
    int bestEval = -MATE_SCORE;

    std::cout << "\nSearching at depth " << depth << "...\n";

    for (const auto& move : moves) {
      board.makeMove(move);
      // already made a move for the maximizing player in the
      // above line so pass false
      int evaluation = minmaxSearch(depth, false);
      std::cout << "  Move: " << uci::moveToUci(move)
                << "  Evaluation: " << evaluation << "\n";
      board.unmakeMove(move);

      if (evaluation > bestEval) {
        bestEval = std::max(evaluation, bestEval);
        bestMove = move;
      }
    }

    std::cout << "\nBest move: " << uci::moveToUci(bestMove)
              << "  Evaluation: " << bestEval
              << "\nPositions analyzed: " << positionsSearched << "\n";
    return bestMove;
  }
}
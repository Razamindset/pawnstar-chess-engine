#include "engine.hpp"

void Engine::orderMoves(Movelist& moves) {
  std::vector<std::pair<Move, int>> scoredMoves;
  scoredMoves.reserve(moves.size());

  for (const auto& move : moves) {
    int score = -800;  // worst queen takes pawn

    // ! If we make n moves on the board for each reacursive call that is
    // computationally heavy If we skip this step the engine will still look
    // at all the moves but 50% performace boost with same result. So we donot
    // need to check for checks and mates here

    // Prioritize captures using MVV-LVA
    if (board.isCapture(move)) {
      score = 0;
      Piece attacker = board.at(move.from());
      Piece victim = board.at(move.to());
      score += getPieceValue(victim) - getPieceValue(attacker);
    }

    // Prioritize promotions

    if (move.promotionType() == QUEEN) score += 900;
    if (move.promotionType() == ROOK) score += 500;
    if (move.promotionType() == BISHOP) score += 320;
    if (move.promotionType() == KNIGHT) score += 300;

    scoredMoves.emplace_back(move, score);
  }

  // Todo Need to learn this sorting magic function ask gpt for now
  // Sort moves by descending score
  std::sort(scoredMoves.begin(), scoredMoves.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });

  // Replace original move list with sorted moves
  moves.clear();
  for (const auto& [move, score] : scoredMoves) {
    moves.add(move);
  }
}

int Engine::negaMax(int depth, int alpha, int beta) {
  positionsSearched++;

  if (isGameOver()) {
    return evaluatePosition(board);
  }

  // Check the tts for matches
  uint64_t hash = board.hash();
  int ttScore = 0;
  Move ttMove = Move::NULL_MOVE;

  if (probeTT(hash, depth, ttScore, alpha, beta, ttMove)) {
    return ttScore;
  }

  if (depth <= 0) {
    int eval = evaluatePosition(board);
    storeTT(hash, 0, eval, TTEntryType::EXACT, Move::NULL_MOVE);
    return eval;
  }

  Movelist moves;
  movegen::legalmoves(moves, board);

  if (moves.empty()) {
    return evaluatePosition(board);
  }

  // If we got a move from TT, try that first
  if (ttMove != Move::NULL_MOVE) {
    // Check for safety if the tt move is in the list
    for (size_t i = 0; i < moves.size(); i++) {
      if (moves[i] == ttMove) {
        std::swap(moves[0], moves[i]);
        break;
      }
    }
  } else {
    orderMoves(moves);
  }

  int maxScore = -MATE_SCORE;  // Should be defined as a very negative number
  Move bestMove = Move::NULL_MOVE;
  TTEntryType entryType = TTEntryType::UPPER;

  for (const auto& move : moves) {
    board.makeMove(move);
    int score = -negaMax(depth - 1, -beta, -alpha);

    // std::cout << "Move: " << uci::moveToUci(move) << " " << score << "\n";
    board.unmakeMove(move);

    if (score > maxScore) {
      maxScore = score;
    }

    if (score > alpha) {
      alpha = score;
      entryType = TTEntryType::EXACT;

      if (alpha >= beta) {
        entryType = TTEntryType::LOWER;
        break;  // Beta cutoff
      }
    }
  }
  storeTT(hash, depth, maxScore, entryType, bestMove);

  return maxScore;
}

std::string Engine::getBestMove(int depth) {
  if (isGameOver()) {
    return "";
  }

  Movelist moves;
  chess::movegen::legalmoves(moves, board);

  if (moves.empty()) {
    return "";
  }

  orderMoves(moves);

  positionsSearched = 0;
  Move bestMove = moves[0];
  int bestScore = -MATE_SCORE;

  for (const auto& move : moves) {
    board.makeMove(move);

    if (isGameOver() && getGameOverReason() == GameResultReason::CHECKMATE) {
      if (board.sideToMove() == Color::WHITE) {
        bestScore = -MATE_SCORE;
        bestMove = move;
      } else {
        bestScore = MATE_SCORE;
        bestMove = move;
      }
      break;
    }

    int score = -negaMax(depth - 1, -MATE_SCORE, MATE_SCORE);
    board.unmakeMove(move);

    if (score > bestScore) {
      bestScore = score;
      bestMove = move;
    }
  }

  return uci::moveToUci(bestMove);
}

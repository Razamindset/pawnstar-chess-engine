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

/* Extend the search to explore tactical possibilites */
int Engine::extendedSearch(int alpha, int beta, int depth, int ply) {
  // Todo add tts to this search too
  positionsSearched++;

  if (depth <= 0) return evaluatePosition(board, ply);

  bool inCheck = board.inCheck();

  int evaluation = evaluatePosition(board, ply);

  // Alpha-beta pruning
  if (!inCheck && evaluation >= beta) return beta;

  // If not in check and eval is better than our current alpha, update
  // alpha
  if (!inCheck) alpha = std::max(evaluation, alpha);

  Movelist moves;
  movegen::legalmoves(moves, board);

  if (moves.empty()) {
    if (inCheck) {
      return -MATE_SCORE + ply;
    } else {
      return 0;
    }
  }

  orderMoves(moves);

  for (const auto& move : moves) {
    //! Following ifs will ensure that we skip bad moves
    // If not in check, we consider captures and promotions
    if (!inCheck && !board.isCapture(move) && move.typeOf() != Move::PROMOTION)
      continue;

    // If we're considering captures and not in check, perform additional
    // pruning
    if (!inCheck && board.isCapture(move)) {
      Piece attacker = board.at(move.from());
      Piece victim = board.at(move.to());

      // Delta pruning - if capturing the most valuable piece wouldn't improve
      // alpha
      const int FUTILITY_MARGIN = 200;
      if (evaluation + getPieceValue(victim) + FUTILITY_MARGIN < alpha)
        continue;

      // Skip captures that lose material (simplified SEE check)
      if (getPieceValue(attacker) > getPieceValue(victim)) {
        // Exception for pawn promotions, which are always worth considering
        if (move.typeOf() != Move::PROMOTION || attacker.type() != PAWN)
          continue;
      }
    }

    board.makeMove(move);
    int score = -extendedSearch(-beta, -alpha, depth - 1, ply + 1);
    board.unmakeMove(move);

    // Beta cutoff
    if (score >= beta) return beta;

    // Update alpha
    alpha = std::max(alpha, score);
  }

  return alpha;
}

int Engine::negaMax(int depth, int alpha, int beta, int ply) {
  positionsSearched++;
  ply++;

  if (isGameOver()) {
    return evaluatePosition(board, ply);
  }

  // Check the tts for matches
  uint64_t hash = board.hash();
  int ttScore = 0;
  Move ttMove = Move::NULL_MOVE;

  if (probeTT(hash, depth, ttScore, alpha, beta, ttMove)) {
    return ttScore;
  }

  if (depth <= 0) {
    int eval = extendedSearch(alpha, beta, 8, ply);
    storeTT(hash, 0, eval, TTEntryType::EXACT, Move::NULL_MOVE);
    return eval;
  }

  Movelist moves;
  movegen::legalmoves(moves, board);

  if (moves.empty()) {
    return evaluatePosition(board, ply);
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
    int score = -negaMax(depth - 1, -beta, -alpha, ply);

    // std::cout << "Move: " << uci::moveToUci(move) << " " << score << "\n";
    board.unmakeMove(move);

    if (score > maxScore) {
      maxScore = score;
      bestMove = move;
    }

    if (score > alpha) {
      alpha = score;
      entryType = TTEntryType::EXACT;
      bestMove = move;

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

    int score = -negaMax(depth - 1, -MATE_SCORE, MATE_SCORE, 1);
    board.unmakeMove(move);

    if (score > bestScore) {
      bestScore = score;
      bestMove = move;
    }
  }

  // std::cout << "Hits in tt: " << ttHits << "\n";

  // ! Fix this uci format mate distance reporting
  // if (std::abs(bestScore) > MATE_SCORE - 100) {  // It's a mate score
  //   int mateDistance;

  //   mateDistance = (MATE_SCORE - bestScore + 1) / 2;
  //   std::cout << "info score mate " << mateDistance << "\n";

  // } else {
  //   // Regular centipawn score
  //   std::cout << "info score cp " << bestScore << "\n";
  // }

  return uci::moveToUci(bestMove);
}

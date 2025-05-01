#include "engine.hpp"

void Engine::orderMoves(Movelist& moves) {
  std::vector<std::pair<Move, int>> scoredMoves;
  scoredMoves.reserve(moves.size());

  for (const auto& move : moves) {
    int score = 0;  // worst queen takes pawn

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
int Engine::extendedSearch(int alpha, int beta, int ply) {
  positionsSearched++;
  ply++;
  int evaluation = evaluatePosition(board, ply);

  // Alpha-beta pruning: If the evaluation is greater than or equal to beta,
  // the minimizing player has found a move that the maximizing player would
  // never allow. So, we prune this branch.
  if (evaluation >= beta) return beta;

  // Update alpha to track the best score found so far for the maximizing
  // player.
  alpha = std::max(evaluation, alpha);

  Movelist moves;
  movegen::legalmoves(moves, board);

  if (moves.empty()) {
    if (board.inCheck()) {
      return -MATE_SCORE + ply;
    } else {
      return 0;
    }
  }

  for (const auto& move : moves) {
    board.makeMove(move);
    bool inCheck = board.inCheck();
    board.unmakeMove(move);
    if (!board.isCapture(move) || !inCheck)
      continue;  // Only consider captures in quiescence search.

    // The following line is really necessary. I donot know if it is the best
    // thing but it works. What we do is only consider capturing moves of
    // higher value with lower value pieces. This brings down our search time
    // from 30s to 6-7s
    Piece attacker = board.at(move.from());
    Piece victim = board.at(move.to());

    // Allow equal captures (like queen takes queen)
    if (getPieceValue(attacker) > getPieceValue(victim) + 50) continue;

    // Consider promoted pawns specially
    if (move.typeOf() == Move::PROMOTION) {
      // Always consider pawn captures that lead to promotion
      if (attacker.type() == PAWN) goto makeMove;
    }

  makeMove:
    board.makeMove(move);
    // Negamax with alpha-beta pruning: The roles of alpha and beta are
    // swapped because each layer alternates between maximizing and
    // minimizing.
    int score = -evaluatePosition(board, ply);
    board.unmakeMove(move);

    // Beta cutoff: If we find a move better than beta for the maximizing
    // player, the minimizing player will never allow this position, so we
    // prune the search.
    if (score >= beta) {
      return beta;
    }

    // Update alpha to the best score found so far.
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
    int eval = extendedSearch(alpha, beta, ply);
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

    int score = -negaMax(depth - 1, -MATE_SCORE, MATE_SCORE, 1);
    board.unmakeMove(move);

    if (score > bestScore) {
      bestScore = score;
      bestMove = move;
    }
  }

  std::cout << "info depth 4 score cp " << bestScore << "\n";

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

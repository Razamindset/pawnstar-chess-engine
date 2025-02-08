#include "engine.hpp"

void Engine::initializeEngine() { std::cout << "Engine Initialized \n"; }

void Engine::setPosition(const std::string& fen) { board = Board(fen); }

void Engine::printBoard() { std::cout << board; }

bool Engine::isGameOver(const Board& board) {
  auto result = board.isGameOver();
  return result.second != GameResult::NONE;
}

GameResultReason Engine::getGameOverReason(const Board& board) {
  auto result = board.isGameOver();
  return result.first;
}

// Get material value of piece type
int Engine::getPieceValue(Piece piece) {
  switch (piece) {
    case PieceGenType::PAWN:
      return 100;
    case PieceGenType::KNIGHT:
      return 300;
    case PieceGenType::BISHOP:
      return 320;
    case PieceGenType::ROOK:
      return 500;
    case PieceGenType::QUEEN:
      return 900;
    default:
      return 0;  // King has no material value
  }
}

void Engine::orderMoves(Movelist& moves) {
  std::vector<std::pair<Move, int>> scoredMoves;
  scoredMoves.reserve(moves.size());

  for (const auto& move : moves) {
    int score = 0;

    // Prioritize checkmate
    board.makeMove(move);
    if (isGameOver(board) &&
        getGameOverReason(board) == GameResultReason::CHECKMATE) {
      score += MATE_SCORE;  // Highest priority for mate
    }

    if (board.inCheck()) {
      score += 1000;  // always check the forcing moves first
    }
    board.unmakeMove(move);

    // Prioritize captures using MVV-LVA
    if (board.isCapture(move)) {
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

  for (Square sq = 0; sq < 64; sq++) {
    Piece piece = board.at(sq);
    if (piece.type() == PieceType::NONE) continue;

    int index =
        (piece.color() == Color::WHITE) ? sq.index() : mirrorIndex(sq.index());
    int squareValue = 0;

    switch (piece.type()) {
      case PAWN:
        squareValue = PAWN_MAP[index];
        break;
      case KNIGHT:
        squareValue = KNIGHT_MAP[index];
        break;
      case BISHOP:
        squareValue = BISHOP_MAP[index];
        break;
      case ROOK:
        squareValue = ROOK_MAP[index];
        break;
      case QUEEN:
        squareValue = QUEEN_MAP[index];
        break;
      case KING:
        squareValue =
            (board.pieces(PieceType::QUEEN, Color::WHITE).count() +
                 board.pieces(PieceType::QUEEN, Color::BLACK).count() ==
             0)
                ? KING_ENDGAME_MAP[index]
                : KING_MAP[index];
        break;
    }

    eval += (piece.color() == Color::WHITE) ? squareValue : -squareValue;
  }

  return eval;
}

int Engine::minmaxSearch(int depth, int alpha, int beta,
                         bool maximizingPlayer) {
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

  orderMoves(moves);

  // alpha is the best eval so far for maximixing and beta for minimizing player
  // If at any point we see that there is a branch worse than alpha or beta
  // we can cut it
  if (maximizingPlayer) {
    // Maximinzing from -ive extreme
    int bestEval = -MATE_SCORE;

    for (const auto& move : moves) {
      board.makeMove(move);
      int evaluation = minmaxSearch(depth - 1, alpha, beta, false);
      board.unmakeMove(move);

      bestEval = std::max(evaluation, bestEval);

      alpha = std::max(alpha, bestEval);
      // beta is always greater than alpha
      // If beta becomes less than alpha means that position is better for the
      // other player which we avoid
      // To avoid that we donot search the bad braches and prune/cut them
      if (beta <= alpha) break;  // **Prune bad branches**
    }

    return bestEval;

  } else {
    // Minimizing from +ive extreme
    int bestEval = MATE_SCORE;

    for (const auto& move : moves) {
      board.makeMove(move);
      int evaluation = minmaxSearch(depth - 1, alpha, beta, true);
      board.unmakeMove(move);

      bestEval = std::min(evaluation, bestEval);
      beta = std::min(beta, bestEval);

      // beta is always greater than alpha
      // Black wants the beta to be less than alpha (minimize)
      // If any branch gives beta greator than alpha we donot need that
      // So we prune it
      if (alpha >= beta) break;  // **Prune bad branches**
    }

    return bestEval;
  }
}

std::string Engine::getBestMove(int depth) {
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

    orderMoves(moves);

    Move bestMove = moves[0];

    // The worst outcome can only result in a mate as we donot have infinty here
    int bestEval =
        board.sideToMove() == Color::WHITE ? -MATE_SCORE : MATE_SCORE;

    std::cout << "\nSearching at depth " << depth << "...\n";

    for (const auto& move : moves) {
      board.makeMove(move);

      // If there is a mate in 1 then imediately return
      if (isGameOver(board)) {
        if (getGameOverReason(board) == GameResultReason::CHECKMATE) {
          return uci::moveToUci(move);
        }
      }

      int evaluation = minmaxSearch(depth, -MATE_SCORE, MATE_SCORE, false);

      std::cout << "  Move: " << uci::moveToUci(move)
                << "  Evaluation: " << evaluation << "\n";

      board.unmakeMove(move);

      if (board.sideToMove() == Color::WHITE) {
        if (evaluation > bestEval) {
          bestEval = evaluation;
          bestMove = move;
        }
      } else {  // Black to move
        if (evaluation < bestEval) {
          bestEval = evaluation;
          bestMove = move;
        }
      }
    }

    std::cout << "\nBest move: " << uci::moveToUci(bestMove)
              << "  Evaluation: " << bestEval
              << "\nPositions analyzed: " << positionsSearched << "\n";
    return uci::moveToUci(bestMove);
  }
  return "null";
}
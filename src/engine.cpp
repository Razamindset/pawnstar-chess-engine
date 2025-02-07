#include "engine.hpp"

void Engine::initializeEngine() { std::cout << "Engine Initialized!\n"; }

void Engine::setPostion(const std::string fen) { board = Board(fen); }

// Check if game has concluded
bool Engine::isGameOver() {
  auto result = board.isGameOver();
  return result.second != GameResult::NONE;
}

// Convert custom PieceType to chess::PieceType
PieceType Engine::getPieceType(PieceGenType piece) {
  switch (piece) {
    case PAWN:
      return chess::PieceType::PAWN;
    case KNIGHT:
      return chess::PieceType::KNIGHT;
    case BISHOP:
      return chess::PieceType::BISHOP;
    case ROOK:
      return chess::PieceType::ROOK;
    case QUEEN:
      return chess::PieceType::QUEEN;
    case KING:
      return chess::PieceType::KING;
    default:
      throw std::invalid_argument("Invalid piece type");
  }
}

// Evaluate position using material balance and game status
int Engine::evaluatePosition(const Board& board) {
  // Check for terminal game states first
  std::pair<GameResultReason, GameResult> result = board.isGameOver();
  if (result.second != GameResult::NONE) {
    if (result.first == GameResultReason::CHECKMATE) {
      return (board.sideToMove() == Color::WHITE) ? -MATE_SCORE : MATE_SCORE;
    }
    return 0;  // Draw conditions
  }

  // Material evaluation constants
  constexpr int PAWN_VALUE = 100;
  constexpr int KNIGHT_VALUE = 300;
  constexpr int BISHOP_VALUE = 320;
  constexpr int ROOK_VALUE = 500;
  constexpr int QUEEN_VALUE = 900;

  // Calculate material advantage
  auto countMaterial = [&](Color color) {
    return board.pieces(PieceType::PAWN, color).count() * PAWN_VALUE +
           board.pieces(PieceType::KNIGHT, color).count() * KNIGHT_VALUE +
           board.pieces(PieceType::BISHOP, color).count() * BISHOP_VALUE +
           board.pieces(PieceType::ROOK, color).count() * ROOK_VALUE +
           board.pieces(PieceType::QUEEN, color).count() * QUEEN_VALUE;
  };

  return countMaterial(Color::WHITE) - countMaterial(Color::BLACK);
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
    // board.makeMove(move);
    // if (isGameOver() &&
    //     board.isGameOver().first == GameResultReason::CHECKMATE) {
    //   score += MATE_SCORE;  // Highest priority for mate
    // }
    // board.unmakeMove(move);

    // Prioritize captures using MVV-LVA
    if (board.isCapture(move)) {
      Piece attacker = board.at(move.from());
      Piece victim = board.at(move.to());
      score += getPieceValue(victim) * 10 - getPieceValue(attacker);
    }

    // Prioritize promotions
    if (move.promotionType() == QUEEN) score += 900;
    if (move.promotionType() == ROOK) score += 500;
    if (move.promotionType() == BISHOP) score += 320;
    if (move.promotionType() == KNIGHT) score += 300;

    scoredMoves.emplace_back(move, score);
  }

  // Sort moves by descending score
  std::sort(scoredMoves.begin(), scoredMoves.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });

  // Replace original move list with sorted moves
  moves.clear();
  for (const auto& [move, score] : scoredMoves) {
    moves.add(move);
  }
}

int Engine::searchAllCaptures(int alpha, int beta) {
  int evaluation = evaluatePosition(board);
  if (evaluation >= beta) return evaluation;
  alpha = std::max(alpha, evaluation);

  Movelist moves;
  movegen::legalmoves(moves, board);

  // Filter for captures only
  Movelist captureMoves;
  for (const auto& move : moves) {
    if (board.isCapture(move)) {
      captureMoves.add(move);
    }
  }

  orderMoves(captureMoves);

  for (const auto& move : captureMoves) {
    board.makeMove(move);
    int ev = -searchAllCaptures(-alpha, -beta);
    board.unmakeMove(move);

    if (ev >= beta) return beta;

    alpha = std::max(alpha, ev);
  }

  return alpha;
}

int Engine::search(int depth, int alpha, int beta) {
  positionsSearched++;

  if (depth == 0 || isGameOver()) {
    return searchAllCaptures(alpha, beta);
  }

  Movelist moves;
  movegen::legalmoves(moves, board);
  orderMoves(moves);

  // Return evaluation if no legal moves
  if (moves.empty()) {
    return evaluatePosition(board);
  }

  int bestEvaluation = -MATE_SCORE;

  for (const auto& move : moves) {
    board.makeMove(move);
    int evaluation = -search(depth - 1, -beta, -alpha);
    board.unmakeMove(move);

    bestEvaluation = std::max(bestEvaluation, evaluation);
    alpha = std::max(alpha, evaluation);

    if (alpha >= beta) {
      break;  // Beta cutoff
    }
  }

  return bestEvaluation;
}
// Find best move using iterative search
std::string Engine::getBestMove(int depth) {
  if (isGameOver()) return "null";

  positionsSearched = 0;  // Reset search counter
  Movelist moves;
  movegen::legalmoves(moves, board);
  if (moves.empty()) return "null";

  Move bestMove = moves[0];
  int bestValue = std::numeric_limits<int>::min();

  std::cout << "\nSearching at depth " << depth << "...\n";

  for (const auto& move : moves) {
    board.makeMove(move);
    // Negate result since next player will minimize our score
    int value = -search(depth - 1, std::numeric_limits<int>::min(),
                        std::numeric_limits<int>::max());
    board.unmakeMove(move);

    std::cout << "  Move: " << uci::moveToUci(move) << "  Evaluation: " << value
              << "\n";

    if (value > bestValue) {
      bestValue = value;
      bestMove = move;
    }
  }

  std::cout << "\nBest move: " << uci::moveToUci(bestMove)
            << "  Evaluation: " << bestValue
            << "\nPositions analyzed: " << positionsSearched << "\n";

  return uci::moveToUci(bestMove);
}
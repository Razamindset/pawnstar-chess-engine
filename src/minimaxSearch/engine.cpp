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

void Engine::storeTTEntry(PackedBoard& board, int depth, int value,
                          NodeType type, Move bestMove) {
  // Always replace with deeper searches or same depth entries
  transpositionTable[board] = {depth, value, type, bestMove};
}

// Todo learn more about this shit. Shitty code keeps getting shitter.
TTEntry* Engine::probeTTEntry(PackedBoard& board, int depth, int alpha,
                              int beta) {
  auto it = transpositionTable.find(board);
  if (it != transpositionTable.end()) {
    TTEntry* entry = &it->second;

    //! God knows what this is
    // Only use entries from equal or deeper searches
    if (entry->depth >= depth) {
      // Value is exact - can be used directly
      if (entry->type == NodeType::EXACT) {
        return entry;
      }

      // Value is an upper bound and fails low - can use for alpha cutoff
      if (entry->type == NodeType::ALPHA && entry->value <= alpha) {
        return entry;
      }

      // Value is a lower bound and fails high - can use for beta cutoff
      if (entry->type == NodeType::BETA && entry->value >= beta) {
        return entry;
      }
    }
  }
  return nullptr;
}

int Engine::minmaxSearch(int depth, int alpha, int beta,
                         bool maximizingPlayer) {
  positionsSearched++;

  // Check for immediate game over
  if (isGameOver(board)) {
    return evaluatePosition(board);
  }

  // At depth 0, return evaluation
  if (depth == 0) {
    return evaluatePosition(board);
  }

  PackedBoard packedBoard = Board::Compact::encode(board);

  // Probe transposition table
  TTEntry* entry = probeTTEntry(packedBoard, depth, alpha, beta);
  if (entry != nullptr) {
    return entry->value;
  }

  Movelist moves;
  movegen::legalmoves(moves, board);

  if (moves.empty()) {
    return evaluatePosition(board);
  }

  orderMoves(moves);

  Move bestMove;
  int originalAlpha = alpha;
  int bestEval = maximizingPlayer ? -MATE_SCORE : MATE_SCORE;

  for (const auto& move : moves) {
    board.makeMove(move);
    int evaluation = minmaxSearch(depth - 1, alpha, beta, !maximizingPlayer);
    board.unmakeMove(move);

    if (maximizingPlayer) {
      if (evaluation > bestEval) {
        bestEval = evaluation;
        bestMove = move;
      }
      alpha = std::max(alpha, bestEval);
    } else {
      if (evaluation < bestEval) {
        bestEval = evaluation;
        bestMove = move;
      }
      beta = std::min(beta, bestEval);
    }

    if (beta <= alpha) {
      break;
    }
  }

  // Determine node type and store position
  NodeType nodeType;
  if (bestEval <= originalAlpha) {
    nodeType = NodeType::ALPHA;
  } else if (bestEval >= beta) {
    nodeType = NodeType::BETA;
  } else {
    nodeType = NodeType::EXACT;
  }

  storeTTEntry(packedBoard, depth, bestEval, nodeType, bestMove);
  return bestEval;
}

std::string Engine::getBestMove(int depth) {
  Movelist moves;
  chess::movegen::legalmoves(moves, board);
  positionsSearched = 0;

  if (moves.empty()) {
    return "null";
  }

  orderMoves(moves);
  Move bestMove = moves[0];

  // For black, we want to maximize negative scores (closer to 0)
  // For white, we want to maximize positive scores
  int bestEval = board.sideToMove() == Color::WHITE ? -MATE_SCORE : MATE_SCORE;

  std::cout << "\nSearching at depth " << depth << "...\n";

  for (const auto& move : moves) {
    board.makeMove(move);

    // The next player will be opposite color, so if current is white,
    // next player (maximizingPlayer) will be false
    int evaluation = minmaxSearch(depth - 1, -MATE_SCORE, MATE_SCORE,
                                  board.sideToMove() == Color::WHITE);

    std::cout << "  Move: " << uci::moveToUci(move)
              << "  Evaluation: " << evaluation << "\n";

    board.unmakeMove(move);

    // For white: pick highest score
    // For black: pick lowest score
    if (board.sideToMove() == Color::WHITE) {
      if (evaluation > bestEval) {
        bestEval = evaluation;
        bestMove = move;
      }
    } else {
      if (evaluation < bestEval) {
        bestEval = evaluation;
        bestMove = move;
      }
    }
  }
  std::cout << "\nBest move: " << uci::moveToUci(bestMove)
            << "  Evaluation: " << bestEval
            << "\nPositions analyzed: " << positionsSearched
            << "\nTransposition table entries: " << getTableSize()
            << "\nTransposition table memory: " << getTableMemoryUsage() / 1024
            << " KB"
            << "\n";

  return uci::moveToUci(bestMove);
}

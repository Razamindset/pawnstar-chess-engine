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

void Engine::orderMoves(Movelist& moves, Move ttMove) {
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

    // The best move from tt should be placed on top
    if (move == ttMove) {
      score += 1000;
    }

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

int Engine::searchAllCaptures(int alpha, int beta) {
  // For now we have refined our function to only check for captures of higher
  // pieces by lower ones to increase performance. If we donot do that our
  // extended search becomes even slower than noraml minmax search
  int evaluation = evaluatePosition(board);
  if (evaluation >= beta) return beta;
  alpha = std::max(alpha, evaluation);

  // We can try extending making this serach faster by using transpotion tables
  PackedBoard packedBoard = Board::Compact::encode(board);

  auto it = transpositionTable.find(packedBoard);

  if (it != transpositionTable.end()) {
    TTEntry entry = it->second;
    if (entry.type == NodeType::EXACT) return entry.value;
    if (entry.type == NodeType::BETA) alpha = std::max(alpha, entry.value);
    if (entry.type == NodeType::ALPHA && entry.value <= alpha) return alpha;
  }

  Movelist moves;
  movegen::legalmoves(moves, board);

  for (const auto& move : moves) {
    if (!board.isCapture(move)) continue;  // Process only captures
    Piece attacker = board.at(move.from());
    Piece victim = board.at(move.to());
    if (getPieceValue(attacker) > getPieceValue(victim)) continue;

    board.makeMove(move);
    int score = -searchAllCaptures(-beta, -alpha);  // Negamax form
    board.unmakeMove(move);

    if (score >= beta) {
      transpositionTable[packedBoard] = {0, score, NodeType::BETA,
                                         Move::NO_MOVE};
      return beta;
    }
    alpha = std::max(alpha, score);
  }

  transpositionTable[packedBoard] = {
      0, alpha, (alpha > evaluation ? NodeType::EXACT : NodeType::ALPHA),
      Move::NO_MOVE};
  return alpha;
}

void Engine::storeTTEntry(PackedBoard& board, int depth, int value,
                          NodeType type, Move bestMove) {
  auto it = transpositionTable.find(board);
  // We will never replace a deepr serach witha shallower
  if (it != transpositionTable.end() && it->second.depth >= depth) {
    return;
  }
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
  // if (depth == 0) {
  //   return evaluatePosition(board);
  // }
  if (depth == 0) {
    return searchAllCaptures(alpha, beta);
  }

  PackedBoard packedBoard = Board::Compact::encode(board);

  // Probe transposition table
  TTEntry* entry = probeTTEntry(packedBoard, depth, alpha, beta);
  if (entry != nullptr) {
    return entry->value;
  }

  Move ttMove = entry ? entry->bestMove : Move::NO_MOVE;

  Movelist moves;
  movegen::legalmoves(moves, board);

  if (moves.empty()) {
    return evaluatePosition(board);
  }

  orderMoves(moves, ttMove);

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
  PackedBoard packedBoard = Board::Compact::encode(board);

  Move bestMove = moves[0];
  orderMoves(moves, bestMove);

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

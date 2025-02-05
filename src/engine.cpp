#include "engine.hpp"

void Engine::initializeEngine() { std::cout << "Enigne Initialized!\n"; }

void Engine::setPostion(const std::string fen) { board = Board(fen); }

bool Engine::isGameOver() {
  std::pair<chess::GameResultReason, chess::GameResult> result =
      board.isGameOver();

  if (result.second != GameResult::NONE) {
    return true;
  }
  return false;
}

chess::PieceType getPieceType(PieceGenType piece) {
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

int Engine::evaluatePosition(const Board& board) {
  int score = 0;

  for (int piece = PAWN; piece <= KING; piece <<= 1) {  // Iterate powers of 2
    score += pieceValues[piece] *
             board
                 .pieces(getPieceType(static_cast<PieceGenType>(piece)),
                         Color::WHITE)
                 .count();
    score -= pieceValues[piece] *
             board
                 .pieces(getPieceType(static_cast<PieceGenType>(piece)),
                         Color::BLACK)
                 .count();
  }

  // Todo Add positional evaluation here later

  return score;
}

int Engine::minmax(int depth, bool maximizingPlayer, int alpha, int beta) {
  // This will be called recursively so we need to add a base case
  if (depth == 0 || isGameOver()) {
    return evaluatePosition(board);
  }
  std::cout << board << "\n";

  std::cout << "Side to move: "
            << (board.sideToMove() == chess::Color::WHITE ? "White" : "Black")
            << depth << maximizingPlayer << std::endl;

  Movelist moves;
  movegen::legalmoves(moves, board);

  for (auto&& move : moves) {
    std::cout << uci::moveToUci(move);
  }

  // So we use a var called maximizing player
  // For understanding we say maximizing=true which means it was white to
  // move For each move white can play we will again call the minimax function
  // recursively and set the maximizing player to false so it is black to move;
  // For each move black can do and so we keep recursion until the required
  // depth is reached

  // Along with each recursion call we need to keep track of the best move
  // possible which will be decided based on the evaluaion fucntion. In future
  // we will extend the evaluation fuction for more better results

  // Alpha tracks the best move for White, while Beta tracks the best move for
  // Black If at any points a branch is found that is worse than beta we can
  // drop it

  if (maximizingPlayer) {
    int maxEval = -100000;

    for (auto&& move : moves) {
      std::cout << "Making move for maximizing player";

      board.makeMove(move);
      int eval = minmax(depth - 1, false, alpha, beta);
      board.unmakeMove(move);

      maxEval = std::max(maxEval, eval);

      alpha = std::max(alpha, eval);

      if (beta <= alpha) {
        // Beta factor starts from +ive infinity and black tries to decrease it
        // while alpha starts from -ive infinty so white tries to increase it If
        // at any point blacks best move becomes smaller or equal to that of
        // white beta. Means that black has a better option than this move
        break;
      }
    }
    return maxEval;
  } else {
    int minEval = 10000;

    for (auto&& move : moves) {
      std::cout << "Making move for minimizing player";
      board.makeMove(move);
      int eval = minmax(depth - 1, true, alpha, beta);
      minEval = std::min(minEval, eval);

      beta = std::min(eval, beta);

      if (alpha >= beta) {
        break;
      }
    }
    return minEval;
  }
}

std::string Engine::getBestMove(int depth) {
  if (isGameOver()) return "null";
  Movelist moves;
  movegen::legalmoves(moves, board);

  if (moves.empty()) return "null";  // Handle case where no moves exist

  int bestValue = -100000;

  Move bestMove = moves[0];

  for (auto&& move : moves) {
    std::cout << "Side to move: "
              << (board.sideToMove() == chess::Color::WHITE ? "White" : "Black")
              << std::endl;
    std::cout << "Move being made: " << uci::moveToUci(move) << std::endl;

    board.makeMove(move);
    int value = minmax(depth - 1, false, -100000, 100000);
    board.unmakeMove(move);

    if (value > bestValue) {
      bestValue = value;
      bestMove = move;
    }
  }
  return uci::moveToUci(bestMove);
}
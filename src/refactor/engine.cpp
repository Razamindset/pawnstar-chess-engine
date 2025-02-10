#include "engine.hpp"

void Engine::initializeEngine() { std::cout << "Enigne Ready!\n"; }

void Engine::setPosition(const std::string& fen) { board.setFen(fen); }

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
    // if (move == ttMove) {
    //   score += 1000;
    // }

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
  if (isGameOver(board)) {
    if (getGameOverReason(board) == GameResultReason::CHECKMATE) {
      return (board.sideToMove() == Color::WHITE) ? MATE_SCORE : -MATE_SCORE;
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

  // Convert to side-to-move perspective for negamax
  return (board.sideToMove() == Color::WHITE) ? eval : -eval;
}

int Engine::negaMax(int depth, int alpha, int beta) {
  // In a simple minimax algorithm we wiil evaluate each position and
  // eventually after evaluting all posssible senarios we get a best move. We
  // We assume that both players play optimally and always choose the best move
  // available to them. For example lets say
  // 1. white plays a move
  // 2. black responds to it
  // 3. and then white responds it.
  // At the lowest level after white responds to it we evalute the position then
  // eval score is given to each of whites moves(at lowest point). White will
  // choose its best move based on evaluations and give it to black. All the
  // middle nodes will recieve a best move from the lower nodes.
  // From all the data middlelayer got from lower layer black now will know what
  // move is most favorable for it to that white can respond and black gets a
  // godd eval score so it will choose that and send it to the top layer. Now
  // white on the top can see and compare all the moves that black can respond
  // with and an evalution score for each of those moves. White chooses the move
  // from the legal moves which results in a postion that most favors white
  // comparitive to other moves it can make.

  // Before explaining the next step one thing should be explained. At initially
  // we say that the worst possible evalution for white is -infinity meaning a
  // mate for black and opposite for black(+ive infinity). We do that becuase we
  // are looking for any move better that results better than a mate for the
  // opponent. For example it is white to move we come accross a move that
  // evalutes to -3 means black is better now but -3 is a better
  // result for white than -ive infinity means black has mate. So after looping
  // through all the possible moves and comparing them we will have the move
  // that has the best evalution form all the legal moves and tries to get the
  // best possible score. The opposite goes if its black to move. Black starts
  // its expectation from +ive infinity so it wants any move that evaluates to
  // less than that. By looping over black will get the best possible move from
  // the set of legal moves. For this example we can say that black is
  // minimizing player and white is maximizing player as white trying to
  // maximize the score and black trying to minimize the score. Hence the name:
  // MINMAX

  //* The saviors Alpha and beta:
  // This seems troubling at start but I hope this example will clear it
  // Think of alpha beta as values that are passed down recursively in the
  // tree. They change at each node and are not shared across branches.Alpha
  // will keep the track of the best evalution for the maximizing player found
  // soo far and beta for the minimizing player. What we are looking for if
  // at any point in a sequence of moves we come across a move that will
  // result in a worse postion than the best score found soo far we can skip
  // all the moves at that branch.

  //* The only key thing to understand here is that alpha will be used to
  //* prune braches when it is black to move or minimizing player to move.
  //* And beta will be used to prune when it is maximizing player to move.

  //? One might ask why don't just skip this move and check other
  //? moves in the branch ? That question troubled me so I want to explain
  //? it.

  //* As in the middle layer black will try to choose the worst possible
  //* move for white obviously in favor of black. Say the first move is worse
  //* than alpha any other move that black will select will be worse than
  //* the first move. So as a whole any move that branch produces for black that
  //* may reach the top layer will never be selected because white will have a
  //* better move at alpha evaluation. So why not just ignore them all
  //* moves!

  //* The opposite now goes for the beta lets say at the some branch where it is
  //* white to move or mX-player to move. Beta needs to be in -ive side or less
  //* than alpha. We know that the best move for black
  //* is of beta evaluation. The above layer of black knows the beta. Say first
  //* eval of first move is greater than beta. Any move choosen from this layer
  //* will will be of more value or equal value to that of first move and more
  //* value than beta. So at the end of the day when all the branches will be
  //* resolved beta will be choosen so our seach at this index does not matter
  //* Seeing that we discard the seach and go our home as not worth doing.

  //* NEGAMAX: the one implented here now

  if (depth == 0) {
    return evaluatePosition(board);
  }

  Movelist moves;
  movegen::legalmoves(moves, board);

  if (moves.empty()) {
    return evaluatePosition(board);
  }

  orderMoves(moves);

  int maxScore = -MATE_SCORE;  // Should be defined as a very negative number

  for (const auto& move : moves) {
    board.makeMove(move);
    int score = -negaMax(depth - 1, -beta, -alpha);
    board.unmakeMove(move);

    if (score > maxScore) {
      maxScore = score;
    }

    alpha = std::max(alpha, score);
    if (alpha >= beta) {
      break;  // Beta cutoff
    }
  }

  return maxScore;
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
  int bestScore = -MATE_SCORE;

  for (const auto& move : moves) {
    board.makeMove(move);
    int score = -negaMax(depth - 1, -MATE_SCORE, MATE_SCORE);
    board.unmakeMove(move);

    if (score > bestScore) {
      bestScore = score;
      bestMove = move;
    }
  }

  std::cout << "Evaluation: " << bestScore << "\n";
  return uci::moveToSan(board, bestMove);
}
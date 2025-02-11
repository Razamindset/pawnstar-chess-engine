#include <chrono>
#include <iostream>
#include <sstream>
#include <vector>

#include "chess.hpp"
#include "engine.hpp"

std::string getReason(GameResultReason result, Board board) {
  switch (result) {
    case GameResultReason::CHECKMATE:
      return board.sideToMove() == Color::WHITE ? "0-1"
                                                : "1-0";  // Losing side to move
    case GameResultReason::STALEMATE:
    case GameResultReason::INSUFFICIENT_MATERIAL:
    case GameResultReason::THREEFOLD_REPETITION:
    case GameResultReason::FIFTY_MOVE_RULE:
      return "1/2-1/2";  // Draw results
    default:
      return "*";  // Game still ongoing
  }
}

int main() {
  Engine engine;
  engine.initializeEngine();
  //   engine.setPosition("5rk1/1pp2ppp/1b3nn1/8/8/1B3NN1/1PP2PPP/5RK1 w - - 0
  //   1");

  Board board;

  std::vector<std::string> moveHistory;  // Store moves in SAN format

  // PGN Metadata
  std::ostringstream pgn;
  pgn << "[Event \"Self-Play Game\"]\n";
  pgn << "[Site \"PawnStar\"]\n";
  pgn << "[Date \"2025.02.09\"]\n";
  pgn << "[Round \"1\"]\n";
  pgn << "[White \"PawnStar\"]\n";
  pgn << "[Black \"PawnStar\"]\n";
  pgn << "[Result \"*\"]\n\n";  // Result to be updated later

  int moveNumber = 1;

  while (true) {
    std::cout << board;

    // Measure time taken for move search
    auto start = std::chrono::high_resolution_clock::now();
    std::string bestMove = engine.getBestMove(4);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    std::cout << "Time taken: " << duration.count() << " seconds\n";

    Movelist moves;
    movegen::legalmoves(moves, board);

    bool movePlayed = false;
    for (const auto& move : moves) {
      if (uci::moveToUci(move) == bestMove) {
        // Convert to SAN notation before making the move
        std::string sanMove = uci::moveToSan(board, move);
        moveHistory.push_back(sanMove);

        board.makeMove(move);
        movePlayed = true;
        break;
      }
    }

    if (!movePlayed) {
      std::cout << "Invalid move generated. Stopping game.\n";
      break;
    }

    // Update PGN notation
    if (board.sideToMove() == Color::BLACK) {
      pgn << moveNumber << ". " << moveHistory.back() << " ";  // White move
    } else {
      pgn << moveHistory.back() << " ";  // Black move
      moveNumber++;
    }

    // Check if the game is over
    if (engine.isGameOver(board)) {
      std::cout << "Game Over!\n";
      break;
    }

    engine.setPosition(board.getFen());
  }

  // Determine and update result in PGN
  GameResultReason result =
      engine.getGameOverReason(board);  // Implement this function
  std::cout << getReason(result, board) << "\n";
  pgn.seekp(0, std::ios::end);  // Move to end and update result
  std::cout << "\nFinal PGN:\n" << pgn.str() << "\n";

  return 0;
}

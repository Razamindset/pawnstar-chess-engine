// #include "chess.hpp"

// using namespace chess;

// int main() {
//   Board board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
//   Movelist moves;
//   std::string imove;

//   std::cout << "Initial Board:\n" << board << "\n";

//   while (true) {
//     movegen::legalmoves(moves, board);  // Generate legal moves
//     for (auto &&legal : moves) {
//       std::cout << uci::moveToUci(legal) << "\t";
//     }

//     std::cout << "\n";

//     std::cout << "Enter move in UCI format (e.g., e2e4): ";
//     std::cin >> imove;

//     bool moveMade = false;
//     for (auto &&move : moves) {
//       if (uci::moveToUci(move) == imove) {
//         board.makeMove(move);
//         moveMade = true;
//         break;  // Exit the loop once a valid move is made
//       }
//     }

//     if (!moveMade) {
//       std::cout << "Invalid move, try again.\n";
//       continue;
//     }

//     std::pair<chess::GameResultReason, chess::GameResult> result =
//         board.isGameOver();

//     if (result.second != GameResult::NONE) {
//       std::cout << "Game Over! ";

//       if (result.first == GameResultReason::CHECKMATE) {
//         std::cout << "Checkmate! ";

//         if (board.sideToMove() == Color::WHITE) {
//           std::cout << "Black Wins.\n";
//         } else {
//           std::cout << "White Wins.\n";
//         }

//       } else if (result.first == GameResultReason::STALEMATE) {
//         std::cout << "Stalemate! It's a Draw.\n";
//       } else if (result.first == GameResultReason::INSUFFICIENT_MATERIAL) {
//         std::cout << "Insufficient Material! It's a Draw.\n";
//       } else if (result.first == GameResultReason::FIFTY_MOVE_RULE) {
//         std::cout << "Fifty-Move Rule! It's a Draw.\n";
//       } else if (result.first == GameResultReason::THREEFOLD_REPETITION) {
//         std::cout << "Threefold Repetition! It's a Draw.\n";
//       }
//       break;
//     }

//     std::cout << "After Board:\n" << board << "\n";
//   }

//   return 0;
// }

#include <iostream>

#include "engine.hpp"

int main() {
  Engine engine;
  engine.initializeEngine();
  std::cout << engine.getBestMove(11);
  return 0;
}
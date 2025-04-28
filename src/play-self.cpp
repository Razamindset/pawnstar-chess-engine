#include <signal.h>

#include <atomic>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include "./engine/engine.hpp"
#include "chess-library/include/chess.hpp"  // Include the chess library

// Configuration
const int SEARCH_DEPTH = 4;  // Depth for engine search
const int MAX_MOVES = 200;   // Max moves before declaring draw
const bool SAVE_PGN = true;  // Whether to save games as PGN
const std::string PGN_FILE = "self_play_games.pgn";

// Global variables to handle crashes
std::vector<std::string> g_currentSanMoves;
std::string g_currentResult = "*";
int g_currentGameNum = 0;
bool g_gameInProgress = false;

// Signal handler for crashes
void signalHandler(int signum) {
  std::cout << "Signal " << signum
            << " received. Saving current game state before exit..."
            << std::endl;

  if (g_gameInProgress && SAVE_PGN) {
    std::ofstream pgnFile(PGN_FILE, std::ios::app);
    if (pgnFile.is_open()) {
      // Create PGN with incomplete game
      std::stringstream pgn;
      pgn << "[Event \"Self-play Game (Interrupted)\"]" << std::endl;
      pgn << "[Site \"Local\"]" << std::endl;

      // Get current date
      auto now = std::chrono::system_clock::now();
      std::time_t time = std::chrono::system_clock::to_time_t(now);
      std::tm* timeinfo = std::localtime(&time);
      char buffer[80];
      std::strftime(buffer, 80, "%Y.%m.%d", timeinfo);

      pgn << "[Date \"" << buffer << "\"]" << std::endl;
      pgn << "[Round \"" << g_currentGameNum << "\"]" << std::endl;
      pgn << "[White \"Pawnstar\"]" << std::endl;
      pgn << "[Black \"Pawnstar\"]" << std::endl;
      pgn << "[Result \"" << g_currentResult << "\"]" << std::endl << std::endl;

      // Directly write the SAN moves we've been storing
      for (size_t i = 0; i < g_currentSanMoves.size(); ++i) {
        if (i % 2 == 0) {
          pgn << (i / 2 + 1) << ". ";
        }

        pgn << g_currentSanMoves[i] << " ";

        // Line break every 5 full moves
        if (i % 10 == 9) {
          pgn << std::endl;
        }
      }

      pgn << g_currentResult << std::endl << std::endl;
      pgnFile << pgn.str();
      pgnFile.close();
      std::cout << "Interrupted game saved to " << PGN_FILE << std::endl;
    }
  }

  exit(signum);
}

// Helper functions for PGN generation
std::string getCurrentDateTime() {
  auto now = std::chrono::system_clock::now();
  std::time_t time = std::chrono::system_clock::to_time_t(now);
  std::tm* timeinfo = std::localtime(&time);

  char buffer[80];
  std::strftime(buffer, 80, "%Y.%m.%d", timeinfo);
  return std::string(buffer);
}

std::string createPgn(const std::vector<std::string>& sanMoves,
                      const std::string& result) {
  std::stringstream pgn;

  // PGN headers
  pgn << "[Event \"Self-play Game\"]" << std::endl;
  pgn << "[Site \"Local\"]" << std::endl;
  pgn << "[Date \"" << getCurrentDateTime() << "\"]" << std::endl;
  pgn << "[Round \"" << g_currentGameNum << "\"]" << std::endl;
  pgn << "[White \"Pawnstar\"]" << std::endl;
  pgn << "[Black \"Pawnstar\"]" << std::endl;
  pgn << "[Result \"" << result << "\"]" << std::endl << std::endl;

  // Write SAN moves directly
  for (size_t i = 0; i < sanMoves.size(); ++i) {
    if (i % 2 == 0) {
      pgn << (i / 2 + 1) << ". ";
    }
    pgn << sanMoves[i] << " ";

    // Line break every 5 full moves
    if (i % 10 == 9) {
      pgn << std::endl;
    }
  }

  pgn << result << std::endl << std::endl;
  return pgn.str();
}

void playSelfGame(int gameNum) {
  std::cout << "=== Starting Game " << gameNum << " ===" << std::endl;

  // Update global game tracking
  g_currentGameNum = gameNum;
  g_currentSanMoves.clear();
  g_currentResult = "*";
  g_gameInProgress = true;

  // Initialize the engine
  Engine engine;
  engine.initilizeEngine();

  // Initialize the chess library board
  chess::Board board;
  std::vector<std::string> sanMoves;
  std::string result = "*";
  int moveCount = 0;

  try {
    // Play game until conclusion
    while (moveCount < MAX_MOVES) {
      moveCount++;

      // Get current FEN from chess library
      std::string currentFen = board.getFen();

      // Set the position in the engine
      engine.setPosition(currentFen);

      // Get best move from engine
      std::string bestMove = engine.getBestMove(SEARCH_DEPTH);

      // Check if game is over
      if (bestMove.empty() || bestMove == "(none)") {
        // Determine if checkmate or stalemate
        bool isInCheck = board.inCheck();
        if (isInCheck) {
          // Checkmate
          result = (moveCount % 2 == 1) ? "0-1" : "1-0";
        } else {
          // Stalemate
          result = "1/2-1/2";
        }
        break;
      }

      Move bst = uci::uciToMove(board, bestMove);

      // Convert UCI move to SAN using chess library
      std::string sanMove = uci::moveToSan(board, bst);

      std::cout << "Move " << moveCount << ": " << bestMove << " (" << sanMove
                << ")" << std::endl;

      // Make the move on the board
      board.makeMove(bst);

      // Store SAN move for PGN
      sanMoves.push_back(sanMove);

      // Update global tracking for crash handling
      g_currentSanMoves = sanMoves;

      // Check for draw conditions (simplified)
      if (board.isHalfMoveDraw() || board.isInsufficientMaterial() ||
          board.isRepetition() || moveCount >= MAX_MOVES) {
        result = "1/2-1/2";
        break;
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "Error during game: " << e.what() << std::endl;
    result = "*";  // Mark as incomplete
  }

  // Update final result for crash handler
  g_currentResult = result;
  std::cout << "Game over: " << result << std::endl;

  // Save game in PGN format if enabled
  if (SAVE_PGN) {
    std::ofstream pgnFile(PGN_FILE, std::ios::app);
    if (pgnFile.is_open()) {
      pgnFile << createPgn(sanMoves, result);
      pgnFile.close();
      std::cout << "Game saved to " << PGN_FILE << std::endl;
    } else {
      std::cerr << "Failed to open PGN file for writing" << std::endl;
    }
  }

  g_gameInProgress = false;
}

int main(int argc, char* argv[]) {
  // Register signal handlers for crash recovery
  signal(SIGINT, signalHandler);
  signal(SIGSEGV, signalHandler);
  signal(SIGTERM, signalHandler);

  int numGames = (argc > 1) ? std::stoi(argv[1]) : 1;

  std::cout << "Starting " << numGames << " self-play game(s)" << std::endl;

  for (int i = 1; i <= numGames; ++i) {
    playSelfGame(i);
  }

  return 0;
}
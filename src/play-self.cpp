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

// Configuration
const int SEARCH_DEPTH = 4;  // Depth for engine search
const int MAX_MOVES = 200;   // Max moves before declaring draw
const bool SAVE_PGN = true;  // Whether to save games as PGN
const std::string PGN_FILE = "self_play_games.pgn";

class MockUCI {
 private:
  Engine engine;
  std::string lastFen;
  std::vector<std::string> moveHistory;

 public:
  MockUCI() { engine.initilizeEngine(); }

  std::string processCommand(const std::string& cmd) {
    std::istringstream iss(cmd);
    std::string token;
    iss >> token;

    if (token == "uci") {
      return "id name Pawnstar\nid author Razamindset\nuciok";
    } else if (token == "isready") {
      return "readyok";
    } else if (token == "position") {
      handlePosition(iss);
      return "";
    } else if (token == "go") {
      return "bestmove " + getBestMove();
    } else if (token == "ucinewgame") {
      engine.initilizeEngine();
      moveHistory.clear();
      return "";
    } else {
      return "";
    }
  }

  const std::vector<std::string>& getMoveHistory() const { return moveHistory; }

 private:
  void handlePosition(std::istringstream& iss) {
    std::string token;
    iss >> token;

    if (token == "startpos") {
      lastFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
      engine.setPosition(lastFen);

      // Process moves if any
      iss >> token;
      if (token == "moves") {
        moveHistory.clear();
        std::string move;
        while (iss >> move) {
          engine.makeMove(move);
          moveHistory.push_back(move);
        }
      }
    } else if (token == "fen") {
      std::string fen;
      // Collect all parts of the FEN string
      for (int i = 0; i < 6; i++) {
        iss >> token;
        fen += token + " ";
      }
      lastFen = fen;
      engine.setPosition(fen);

      // Process moves if any
      iss >> token;
      if (token == "moves") {
        moveHistory.clear();
        std::string move;
        while (iss >> move) {
          engine.makeMove(move);
          moveHistory.push_back(move);
        }
      }
    }
  }

  std::string getBestMove() { return engine.getBestMove(SEARCH_DEPTH); }
};

// Helper functions for PGN generation
std::string getCurrentDateTime() {
  auto now = std::chrono::system_clock::now();
  std::time_t time = std::chrono::system_clock::to_time_t(now);
  std::tm* timeinfo = std::localtime(&time);

  char buffer[80];
  std::strftime(buffer, 80, "%Y.%m.%d", timeinfo);
  return std::string(buffer);
}

std::string uciToSan(const std::string& uciMove) {
  // This is a simplified conversion - for proper SAN you'd need full board
  // state
  return uciMove;
}

std::string createPgn(const std::vector<std::string>& moves,
                      const std::string& result) {
  std::stringstream pgn;

  // PGN headers
  pgn << "[Event \"Self-play Game\"]" << std::endl;
  pgn << "[Site \"Local\"]" << std::endl;
  pgn << "[Date \"" << getCurrentDateTime() << "\"]" << std::endl;
  pgn << "[Round \"1\"]" << std::endl;
  pgn << "[White \"Pawnstar\"]" << std::endl;
  pgn << "[Black \"Pawnstar\"]" << std::endl;
  pgn << "[Result \"" << result << "\"]" << std::endl << std::endl;

  // Moves in SAN format
  for (size_t i = 0; i < moves.size(); ++i) {
    if (i % 2 == 0) {
      pgn << (i / 2 + 1) << ". ";
    }
    pgn << uciToSan(moves[i]) << " ";

    // Line break every 5 full moves
    if (i % 10 == 9) {
      pgn << std::endl;
    }
  }

  pgn << result << std::endl << std::endl;
  return pgn.str();
}

// Detect game end conditions
bool isCheckmate(const std::string& bestMove, const Engine& engine) {
  // In a real implementation, you would check the engine's state
  // For now, we just consider no legal moves as checkmate if not stalemate
  return bestMove.empty() || bestMove == "(none)";
}

bool isDraw(int moveCount, const std::vector<std::string>& moves) {
  if (moveCount >= MAX_MOVES) return true;

  // Check for threefold repetition (simplified)
  // In a real implementation, you would check the actual positions

  return false;
}

void playSelfGame(int gameNum) {
  std::cout << "=== Starting Game " << gameNum << " ===" << std::endl;

  MockUCI engine;

  // Initialize game
  engine.processCommand("ucinewgame");
  engine.processCommand("position startpos");

  std::vector<std::string> allMoves;
  std::string result = "*";

  // Play moves until game ends
  for (int moveCount = 1; moveCount <= MAX_MOVES; ++moveCount) {
    // Get current position with moves
    std::stringstream posCmd;
    posCmd << "position startpos";
    if (!allMoves.empty()) {
      posCmd << " moves";
      for (const auto& move : allMoves) {
        posCmd << " " << move;
      }
    }
    engine.processCommand(posCmd.str());

    // Get best move
    std::string response =
        engine.processCommand("go depth " + std::to_string(SEARCH_DEPTH));

    // Extract move from "bestmove e2e4" format
    std::string bestMove;
    std::istringstream iss(response);
    std::string token;
    iss >> token >> bestMove;

    std::cout << "Move " << moveCount << ": " << bestMove << std::endl;

    // Check for game end
    if (bestMove.empty() || bestMove == "(none)") {
      // Game ended, determine whether checkmate or stalemate
      result = (moveCount % 2 == 1) ? "0-1" : "1-0";  // Side to move lost
      std::cout << "Game over: " << result << std::endl;
      break;
    }

    // Check for draws
    if (isDraw(moveCount, allMoves)) {
      result = "1/2-1/2";
      std::cout << "Game over: Draw" << std::endl;
      break;
    }

    // Add move to history
    allMoves.push_back(bestMove);
  }

  // Save game in PGN format if enabled
  if (SAVE_PGN) {
    std::ofstream pgnFile(PGN_FILE, std::ios::app);
    if (pgnFile.is_open()) {
      pgnFile << createPgn(allMoves, result);
      pgnFile.close();
      std::cout << "Game saved to " << PGN_FILE << std::endl;
    }
  }
}

int main(int argc, char* argv[]) {
  int numGames = (argc > 1) ? std::stoi(argv[1]) : 1;

  std::cout << "Starting " << numGames << " self-play game(s)" << std::endl;

  for (int i = 1; i <= numGames; ++i) {
    playSelfGame(i);
  }

  return 0;
}
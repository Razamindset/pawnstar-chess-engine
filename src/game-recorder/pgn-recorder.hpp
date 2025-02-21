#ifndef GAME_RECORDER_HPP
#define GAME_RECORDER_HPP

//* I did not write this cause i am making an engine right now and that's all i
// can focus now for

//! Ai generated shit always have error for now just dont use the pgn genrator I
//! will fix it later

#include <chrono>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "../chess-library/include/chess.hpp"
#include "../engine/engine.hpp"
#include "game-metadata.hpp"

class GameRecorder {
 private:
  Engine& whiteEngine;
  Engine& blackEngine;
  Board board;
  std::vector<std::string> moveHistory;
  std::chrono::system_clock::time_point gameStart;

  GameMetadata metadata;

  std::string getGameResult(GameResultReason result) {
    switch (result) {
      case GameResultReason::CHECKMATE:
        return board.sideToMove() == Color::WHITE ? "0-1" : "1-0";
      case GameResultReason::STALEMATE:
      case GameResultReason::INSUFFICIENT_MATERIAL:
      case GameResultReason::THREEFOLD_REPETITION:
      case GameResultReason::FIFTY_MOVE_RULE:
        return "1/2-1/2";
      default:
        return "*";
    }
  }

  std::string getCurrentDate() {
    auto now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    std::tm* tm = std::localtime(&time);
    char date[11];
    std::strftime(date, sizeof(date), "%Y.%m.%d", tm);
    return std::string(date);
  }

 public:
  GameRecorder(Engine& white, Engine& black)
      : whiteEngine(white), blackEngine(black) {
    gameStart = std::chrono::system_clock::now();
  }

  void setMetadata(const GameMetadata& meta) { metadata = meta; }

  void setInitialPosition(const std::string& fen) {
    board.setFen(fen);
    whiteEngine.setPosition(fen);
    blackEngine.setPosition(fen);
  }

  std::string generatePGN() {
    std::ostringstream pgn;

    // Write headers
    pgn << "[Event \"" << metadata.event << "\"]\n";
    pgn << "[Site \"" << metadata.site << "\"]\n";
    pgn << "[Date \"" << getCurrentDate() << "\"]\n";
    pgn << "[Round \"" << metadata.round << "\"]\n";
    pgn << "[White \"" << metadata.white << "\"]\n";
    pgn << "[Black \"" << metadata.black << "\"]\n";
    pgn << "[FEN \"" << board.getFen() << "\"]\n";

    // Add result at the end
    GameResultReason result = whiteEngine.getGameOverReason(board);
    std::string gameResult = getGameResult(result);
    pgn << "[Result \"" << gameResult << "\"]\n\n";

    // Write moves
    int moveNumber = 1;
    for (size_t i = 0; i < moveHistory.size(); i++) {
      if (i % 2 == 0) {
        pgn << moveNumber << ". ";
        moveNumber++;
      }
      pgn << moveHistory[i] << " ";
    }

    pgn << gameResult;
    return pgn.str();
  }

  void playGame() {
    while (true) {
      if (board.isGameOver().first != GameResultReason::NONE) break;

      Engine& currentEngine =
          (board.sideToMove() == Color::WHITE) ? whiteEngine : blackEngine;

      // Get and time the move
      auto start = std::chrono::high_resolution_clock::now();
      // std::cout << metadata.searchDepth << "\n";
      std::string bestMove = currentEngine.getBestMove(metadata.searchDepth);
      auto end = std::chrono::high_resolution_clock::now();
      auto duration =
          std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

      std::cout << "Move generated in: " << duration.count() / 1000.0
                << " seconds\n";

      // Make the move
      Movelist moves;
      movegen::legalmoves(moves, board);
      bool movePlayed = false;

      for (const auto& move : moves) {
        if (uci::moveToUci(move) == bestMove) {
          std::string sanMove = uci::moveToSan(board, move);
          moveHistory.push_back(sanMove);

          board.makeMove(move);
          whiteEngine.opponentMoves(move);
          blackEngine.opponentMoves(move);

          std::cout << "Move played: " << sanMove << "\n";
          std::cout << board;

          movePlayed = true;
          break;
        }
      }

      if (!movePlayed) {
        std::cout << "Invalid move generated. Stopping game.\n";
        break;
      }
    }
  }

  void savePGN(const std::string& filename) {
    std::ofstream file(filename);
    file << generatePGN();
    file.close();
  }
};

#endif
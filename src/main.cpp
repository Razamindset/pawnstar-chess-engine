#include "./game-recorder/pgn-recorder.hpp"

int main() {
  // Initialize engines
  Engine whiteEngine;
  Engine blackEngine;
  whiteEngine.initializeEngine();
  blackEngine.initializeEngine();

  // Create game recorder
  GameRecorder recorder(whiteEngine, blackEngine);

  // Set up metadata
  GameMetadata meta;
  meta.event = "Test Position";
  meta.searchDepth = 4;
  meta.startingFen = "3K4/3P4/8/8/8/8/3qk3/8 w - - 0 1";

  // Configure the recorder
  recorder.setMetadata(meta);

  // Play the game
  recorder.playGame();

  // Save the PGN
  recorder.savePGN("test_game.pgn");

  return 0;
}
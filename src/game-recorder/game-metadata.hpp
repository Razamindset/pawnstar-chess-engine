#ifndef GAME_METADATA_HPP
#define GAME_METADATA_HPP

#include <string>

struct GameMetadata {
  std::string event = "Self-Play Game";
  std::string site = "PawnStar";
  std::string white = "PawnStar";
  std::string black = "PawnStar";
  std::string round = "1";
  int searchDepth = 4;
  std::string startingFen =
      "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
};

#endif
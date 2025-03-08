#include "engine.hpp"

void Engine::clearTranspositionTable() {
  transpositionTable.clear();
  ttHits = 0;
}

bool Engine::probeTT(uint64_t hash, int depth, int& score, int alpha, int beta,
                     Move& bestMove) {
  auto it = transpositionTable.find(hash);
  if (it == transpositionTable.end()) {
    return false;
  }
  const TTEntry& entry = it->second;

  if (entry.depth >= depth) {
    score = entry.score;
    bestMove = entry.bestMove;

    // Handle differnt entry types
    if (entry.type == TTEntryType::EXACT) {
      score = entry.score;
      return true;
    } else if (entry.type == TTEntryType::LOWER && entry.score >= beta) {
      score = entry.score;
      return true;
    } else if (entry.type == TTEntryType::UPPER && entry.score <= alpha) {
      score = entry.score;
      return true;
    }
  } else {
    // If insufficient depth we can still use the best move
    bestMove = entry.bestMove;
  }
  return false;
}

void Engine::storeTT(uint64_t hash, int depth, int score, TTEntryType type,
                     Move bestMove) {
  // keep the table of a constant size
  if (transpositionTable.size() > 1000000) {
    // Todo implement a replacement scheme
    if (transpositionTable.size() > 2000000) {
      clearTranspositionTable();
    }
  }

  TTEntry entry;
  entry.hash = hash;
  entry.score = score;
  entry.depth = depth;
  entry.bestMove = bestMove;
  entry.type = type;

  transpositionTable[hash] = entry;
}
#include "engine.hpp"

void Engine::printBoard() { std::cout << board; }

void Engine::setPosition(const std::string& fen) { board.setFen(fen); }
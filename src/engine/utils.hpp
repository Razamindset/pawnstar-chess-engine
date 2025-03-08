#ifndef UTILS_HPP
#define UTILS_HPP

constexpr int MATE_SCORE = 100000;

constexpr int mirrorIndex(int sq) { return (7 - sq / 8) * 8 + (sq % 8); }

#endif
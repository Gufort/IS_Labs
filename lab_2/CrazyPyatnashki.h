// CrazyPyatnashki.h
#pragma once          // или #ifndef-гард
#include <string>
#include <vector>
#include <cstdint>

using u64 = uint64_t;

constexpr int BOARD_SIZE  = 16;
constexpr int BOARD_WIDTH = 4;
constexpr uint64_t TARGET = 0x123456789ABCDEF0ULL;

u64  parseState(const std::string& input);
int  getNibble(u64 state, int pos);
u64  setNibble(u64 state, int pos, int value);
int  findBlankPosition(u64 state);
u64  shiftBlankPosition(u64 state, int blankPosition, int newBlankPosition);
bool isSolvable(u64 state);
std::vector<u64> getNeighbours(u64 state);

extern int  newBlankTable[BOARD_SIZE][4];
extern bool isMovesInit;
void initializeMoves();
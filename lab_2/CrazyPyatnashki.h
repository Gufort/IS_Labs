// CrazyPyatnashki.h
#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <ctime>
#include <random>
#include <stdexcept>
#include <queue>
#include "custom_map.h"
#include "custom_set.h"

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
std::string stateToString(uint64_t state);
u64 parseState(const std::string& input);

void initManhattan();
int  heuristic(u64 state);

int bfs(u64 state);
int dfs(u64 state, int maxDepth);
int helpDfs(u64 currentState, int depth, int limit, custom_set& visited);
int ids(u64 state);
bool helpIds(u64 current, int depth, int limit, custom_set& visited);
int astar(u64 state);
int idaStar(u64 state);
int helpIdaStar(u64 current, int g, int threshold, custom_set& visited, int& nextThreshold);
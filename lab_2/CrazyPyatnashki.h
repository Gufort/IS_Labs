#ifndef IS_LABS_CRAZYPYATNASHKI_H
#define IS_LABS_CRAZYPYATNASHKI_H

#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <queue>
#include <unordered_map>
#include <cmath>
#include <stdexcept>
#include <algorithm>
#include <cstdlib>

const std::string TARGET = "123456789ABCDEF0";

bool isSolvable(const std::string& input);
int countOfInversions(const std::string& input, char blank);
std::string generateSequence(int width, int steps);
std::vector<std::string> getNeighbours(const std::string& input);
int bfs(const std::string& input);
int dfs(const std::string& input);
bool helpDfs(const std::string& current, int depth, int limit, std::unordered_set<std::string>& visited);

#endif //IS_LABS_CRAZYPYATNASHKI_H

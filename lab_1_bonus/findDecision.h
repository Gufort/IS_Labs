#ifndef IS_LABS_SIMPLE_TEST_H
#define IS_LABS_SIMPLE_TEST_H
#include <random>
#include <vector>
#include <array>
#include <iostream>
#include <chrono>

const int BOARD_SIZE = 6;
const int DOMINO_COUNT = 28;
const int USED_DOMINOES = 18;
const int TARGET = 13;

struct Domino
{
    int a;
    int b;
};

int calculateScore(const std::array<int,
    DOMINO_COUNT>& order, const std::array<bool,
    USED_DOMINOES>& flipped, std::array<int, BOARD_SIZE * BOARD_SIZE>* result = nullptr);

bool simulatedAnnealing(std::array<int, DOMINO_COUNT>& bestOrder,
                        std::array<bool, USED_DOMINOES>& bestFlipped);

void printResult(const std::array<int, DOMINO_COUNT>& order, const std::array<bool, USED_DOMINOES>& flipped);

extern std::vector<Domino> dominoes;
extern std::mt19937 generator;

#endif



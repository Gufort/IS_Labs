#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <random>

#include "CrazyPyatnashki.h"

const int LIMIT_OF_STEPS = 20;

int main()
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // std::vector<std::string> generatedStrings;
    //
    // for (int i = 0; i < 50; ++i)
    // {
    //     generatedStrings.push_back(generateSequence(4, 10 + std::rand() % LIMIT_OF_STEPS));
    //     std::cout << i + 1 << ": " << generatedStrings.back() << std::endl;
    // }
    std::vector<std::string> generatedStrings;
    generatedStrings.push_back("16245A3709C8DEBF");

    // std::cout << '\n' << "==> BFS " << std::endl;
    // for (int i = 0; i < 10; ++i)
    // {
    //     auto start = std::chrono::steady_clock::now();
    //     int steps = bfs(generatedStrings[i]);
    //     auto end = std::chrono::steady_clock::now();
    //     auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    //     std::cout << "Sequence " << i + 1 << ": " << generatedStrings[i]
    //               << " | solution found in " << steps << " steps"
    //               << " | time: " << elapsedMs << " ms" << std::endl;
    // }

    std::cout << '\n' << "==> DFS " << std::endl;
    for (int i = 0; i < 10; ++i)
    {
        auto start = std::chrono::steady_clock::now();
        int steps = dfs(generatedStrings[i]);
        auto end = std::chrono::steady_clock::now();
        auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "Sequence " << i + 1 << ": " << generatedStrings[i]
                  << " | solution found in " << steps << " steps"
                  << " | time: " << elapsedMs << " ms" << std::endl;
    }

    return 0;
}
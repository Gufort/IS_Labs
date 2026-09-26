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
    std::vector<std::string> generatedStrings = {
        "123456789ABCDEF0",
        "16245A3709C8DEBF",
        "7023168459ACDEBF",
        "FE169B4C0A73D852"
    };

    // std::cout << '\n' << "==> BFS " << std::endl;
    // for (int i = 0; i < 4; ++i)
    // {
    //     auto start = std::chrono::steady_clock::now();
    //     int steps = bfs(parseState(generatedStrings[i]));
    //     auto end = std::chrono::steady_clock::now();
    //     auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    //     std::cout << "Sequence " << i + 1 << ": " << generatedStrings[i]
    //               << " | solution found in " << steps << " steps"
    //               << " | time: " << elapsedMs << " ms" << std::endl;
    // }

    // std::cout << '\n' << "==> DFS " << std::endl;
    // for (int i = 0; i < 10; ++i)
    // {
    //     auto start = std::chrono::steady_clock::now();
    //     int steps = dfs(parseState(generatedStrings[i]), 50);
    //     auto end = std::chrono::steady_clock::now();
    //     auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    //     std::cout << "Sequence " << i + 1 << ": " << generatedStrings[i]
    //               << " | solution found in " << steps << " steps"
    //               << " | time: " << elapsedMs << " ms" << std::endl;
    // }

    std::cout << '\n' << "==> IDS " << std::endl;
    for (int i = 0; i < 4; ++i)
    {
        auto start = std::chrono::steady_clock::now();
        int steps = ids(parseState(generatedStrings[i]));
        auto end = std::chrono::steady_clock::now();
        auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "Sequence " << i + 1 << ": " << generatedStrings[i]
                  << " | solution found in " << steps << " steps"
                  << " | time: " << elapsedMs << " ms" << std::endl;
    }

    std::cout << '\n' << "==> A* " << std::endl;
    for (int i = 0; i < 4; ++i)
    {
        auto start = std::chrono::steady_clock::now();
        int steps = astar(parseState(generatedStrings[i]));
        auto end = std::chrono::steady_clock::now();
        auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "Sequence " << i + 1 << ": " << generatedStrings[i]
                  << " | solution found in " << steps << " steps"
                  << " | time: " << elapsedMs << " ms" << std::endl;
    }

    std::cout << '\n' << "==> IDA* " << std::endl;
    for (int i = 0; i < 4; ++i)
    {
        auto start = std::chrono::steady_clock::now();
        int steps = idaStar(parseState(generatedStrings[i]));
        auto end = std::chrono::steady_clock::now();
        auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "Sequence " << i + 1 << ": " << generatedStrings[i]
                  << " | solution found in " << steps << " steps"
                  << " | time: " << elapsedMs << " ms" << std::endl;
    }

    return 0;
}


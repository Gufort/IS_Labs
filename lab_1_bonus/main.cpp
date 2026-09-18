#include <iostream>
#include <vector>
#include <array>
#include <algorithm>
#include <random>
#include <chrono>
#include <cmath>
#include <numeric>
#include <iomanip>
#include "findDecision.h"

using namespace std;

int main()
{
    setlocale(LC_ALL, "rus");

    for (int a = 0; a <= 6; a++)
        for (int b = a; b <= 6; b++)
            dominoes.push_back({ a, b });

    array<int, DOMINO_COUNT> bestOrder{};
    array<bool, USED_DOMINOES> bestFlipped{};
    auto start = chrono::steady_clock::now();
    bool found = simulatedAnnealing(bestOrder, bestFlipped);
    auto finish = chrono::steady_clock::now();
    double milliseconds = chrono::duration<double, milli>(finish - start).count();

    if (!found){
        cout << "Решение не найдено." << endl;
        return 0;
    }

    cout << "\nРешение найдено!\n";
    printResult(bestOrder, bestFlipped);
    cout << "Время поиска: " << milliseconds << " мс\n";
    return 0;
}
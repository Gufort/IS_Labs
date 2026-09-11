#include <vector>
#include "iostream"
#include "simple_test.h"
#include "format"
#include <chrono>

using namespace std;

int main()
{
    setlocale(LC_ALL, "rus");
    vector<pair<int, int>> testData = {
        {1, 100},
        {2, 55},
        {2, 100},
        {1, 97},
        {2, 1000},
        {2, 10000001}
    };

    // 1
    cout << "==> Решение для пункта 1" << endl;
    cout << format("|{:-<17}|{:-<15}|{:-<21}|{:-<15}|\n", "", "", "", "");
    cout << format("| {:<15} | {:<13} | {:>19} | {:>13} |\n","Начальное число","Искомое число","Количество операций","Время (мкс)");
    cout << format("|{:-<17}|{:-<15}|{:-<21}|{:-<15}|\n", "", "", "", "");

    for (auto test : testData)
    {
        auto [start, end] = test;
        auto begin = chrono::high_resolution_clock::now();
        int result = findMinSecOfOperation(start, end);
        auto finish = chrono::high_resolution_clock::now();
        auto time = chrono::duration_cast<chrono::microseconds>(finish - begin).count();

        cout << format("| {:<15} | {:<13} | {:>19} | {:>13} |\n",start,end,result,time);

        cout << format("|{:-<17}|{:-<15}|{:-<21}|{:-<15}|\n",
                       "", "", "", "");
    }

    // 2
    cout << endl;
    cout << "==> Решение для пункта 2" << endl;
    cout << format("|{:-<17}|{:-<15}|{:-<21}|{:-<15}|\n", "", "", "", "");
    cout << format("| {:<15} | {:<13} | {:>19} | {:>13} |\n","Начальное число","Искомое число","Количество операций","Время (мкс)");
    cout << format("|{:-<17}|{:-<15}|{:-<21}|{:-<15}|\n", "", "", "", "");

    for (auto test : testData)
    {
        auto [start, end] = test;
        auto begin = chrono::high_resolution_clock::now();
        int result = findMinSecOfThreeOperation(start, end);
        auto finish = chrono::high_resolution_clock::now();
        auto time = chrono::duration_cast<chrono::microseconds>(finish - begin).count();

        cout << format("| {:<15} | {:<13} | {:>19} | {:>13} |\n",start,end,result,time);

        cout << format("|{:-<17}|{:-<15}|{:-<21}|{:-<15}|\n",
                       "", "", "", "");
    }

    // 3
    cout << endl;
    cout << "==> Решение для пункта 3" << endl;
    cout << format("|{:-<17}|{:-<15}|{:-<21}|{:-<15}|\n", "", "", "", "");
    cout << format("| {:<15} | {:<13} | {:>19} | {:>13} |\n","Начальное число","Искомое число","Количество операций","Время (мкс)");
    cout << format("|{:-<17}|{:-<15}|{:-<21}|{:-<15}|\n", "", "", "", "");

    for (auto test : testData)
    {
        auto [start, end] = test;
        auto begin = chrono::high_resolution_clock::now();
        int result = findMinSecOfOperationReverse(start, end);
        auto finish = chrono::high_resolution_clock::now();
        auto time = chrono::duration_cast<chrono::microseconds>(finish - begin).count();

        cout << format("| {:<15} | {:<13} | {:>19} | {:>13} |\n",start,end,result,time);

        cout << format("|{:-<17}|{:-<15}|{:-<21}|{:-<15}|\n",
                       "", "", "", "");
    }

    // 4
    cout << endl;
    cout << "==> Решение для пункта 4" << endl;
    cout << format("|{:-<17}|{:-<15}|{:-<21}|{:-<15}|\n", "", "", "", "");
    cout << format("| {:<15} | {:<13} | {:>19} | {:>13} |\n","Начальное число","Искомое число","Количество операций","Время (мкс)");
    cout << format("|{:-<17}|{:-<15}|{:-<21}|{:-<15}|\n", "", "", "", "");

    for (auto test : testData)
    {
        auto [start, end] = test;
        auto begin = chrono::high_resolution_clock::now();
        int result = findMinSecOfOperationBidirectional(start, end);
        auto finish = chrono::high_resolution_clock::now();
        auto time = chrono::duration_cast<chrono::microseconds>(finish - begin).count();

        cout << format("| {:<15} | {:<13} | {:>19} | {:>13} |\n",start,end,result,time);

        cout << format("|{:-<17}|{:-<15}|{:-<21}|{:-<15}|\n",
                       "", "", "", "");
    }
}
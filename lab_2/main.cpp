#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <chrono>
#include <random>
#include <future>
#include <thread>
#include <cstdio>

#include "CrazyPyatnashki.h"

static constexpr int TIMEOUT_MS = 15'000;

struct Result {
    int    steps   = -1;
    double ms      = 0.0;
    bool   ok      = false;
    bool   timeout = false;
};

template <typename Fn>
static Result runOne(const std::string& input, Fn fn)
{
    Result r;
    u64 state = parseState(input);
    if (!isSolvable(state)) return r;

    auto task = [state, fn]() -> std::pair<int, double> {
        auto t0 = std::chrono::steady_clock::now();
        int steps = fn(state);
        auto t1 = std::chrono::steady_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        return {steps, ms};
    };

    auto pt  = std::make_shared<std::packaged_task<std::pair<int,double>()>>(task);
    auto fut = pt->get_future();

    std::thread worker([pt]() { (*pt)(); });

    if (fut.wait_for(std::chrono::milliseconds(TIMEOUT_MS))
        == std::future_status::ready)
    {
        auto [steps, ms] = fut.get();
        r.steps = steps;
        r.ms    = ms;
        r.ok    = (steps >= 0);
        worker.join();
    }
    else
    {
        worker.detach();
        r.timeout = true;
    }
    return r;
}

static void printHeader()
{
    std::cout << '+' << std::string(4, '-')
              << '+' << std::string(18, '-')
              << '+' << std::string(10, '-')
              << '+' << std::string(13, '-')
              << '+' << std::string(13, '-')
              << '+' << std::string(13, '-')
              << '+' << std::string(13, '-')
              << '+' << '\n';

    std::cout << '|' << std::setw(4)  << " # "
              << '|' << std::setw(18) << " State "
              << '|' << std::setw(10) << " Optimal "
              << '|' << std::setw(13) << " IDS "
              << '|' << std::setw(13) << " A* "
              << '|' << std::setw(13) << " IDA* "
              << '|' << std::setw(13) << " Best "
              << '|' << '\n';

    std::cout << '+' << std::string(4, '-')
              << '+' << std::string(18, '-')
              << '+' << std::string(10, '-')
              << '+' << std::string(13, '-')
              << '+' << std::string(13, '-')
              << '+' << std::string(13, '-')
              << '+' << std::string(13, '-')
              << '+' << '\n';
}

static void printRow(int idx, const std::string& state,
                     int optimal,
                     const Result& ids,
                     const Result& astar,
                     const Result& ida)
{
    auto printStepsAndTime = [&](const Result& r) {
        std::string cell;
        if (r.timeout)   cell = " > 15s";
        else if (!r.ok)  cell = " - ";
        else {
            char buf[32];
            std::snprintf(buf, sizeof(buf), "%d / %.0fms", r.steps, r.ms);
            cell = buf;
        }
        std::cout << '|' << std::setw(13) << cell;
    };

    std::string best = "-";
    double bestMs = 1e18;
    if (ids.ok   && !ids.timeout   && ids.ms   < bestMs) { bestMs = ids.ms;   best = "IDS";  }
    if (astar.ok && !astar.timeout && astar.ms < bestMs) { bestMs = astar.ms; best = "A*";   }
    if (ida.ok   && !ida.timeout   && ida.ms   < bestMs) { bestMs = ida.ms;   best = "IDA*"; }

    std::cout << '|' << std::setw(4)  << idx
              << '|' << std::setw(18) << state
              << '|' << std::setw(10) << optimal;

    printStepsAndTime(ids);
    printStepsAndTime(astar);
    printStepsAndTime(ida);

    std::cout << '|' << std::setw(13) << best
              << '|' << '\n';
}

int main()
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    std::vector<std::string> states = {
        "123456789ABCDEF0",
        "16245A3709C8DEBF",
        "7023168459ACDEBF",
        "FE169B4C0A73D852",
    };

    printHeader();

    for (size_t i = 0; i < states.size(); ++i) {
        const std::string& s = states[i];
        u64 state = parseState(s);

        if (!isSolvable(state)) {
            std::cout << '|' << std::setw(4) << (i + 1)
                      << '|' << std::setw(18) << s
                      << '|' << std::setw(10) << "-"
                      << '|' << std::setw(13) << " unsolvable "
                      << '|' << std::setw(13) << " - "
                      << '|' << std::setw(13) << " - "
                      << '|' << std::setw(13) << " - "
                      << '|' << '\n';

            std::cout << '+' << std::string(4, '-')
                      << '+' << std::string(18, '-')
                      << '+' << std::string(10, '-')
                      << '+' << std::string(13, '-')
                      << '+' << std::string(13, '-')
                      << '+' << std::string(13, '-')
                      << '+' << std::string(13, '-')
                      << '+' << '\n';
            continue;
        }

        int optimal = (heuristic(state) <= 20) ? bfs(state) : -1;

        Result rIds = runOne(s, ids);
        Result rA   = runOne(s, astar);
        Result rIda = runOne(s, idaStar);

        printRow(static_cast<int>(i + 1), s, optimal, rIds, rA, rIda);

        std::cout << '+' << std::string(4, '-')
                  << '+' << std::string(18, '-')
                  << '+' << std::string(10, '-')
                  << '+' << std::string(13, '-')
                  << '+' << std::string(13, '-')
                  << '+' << std::string(13, '-')
                  << '+' << std::string(13, '-')
                  << '+' << '\n';
    }

    return 0;
}
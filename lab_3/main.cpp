#include "BabylonTower.h"
#include <iomanip>
#include <sstream>
constexpr std::array<State, 10> startStates = {
    State{
        '.',
        'r','w','g','b','y',
        'r','w','g','b','y',
        'r','w','g','b','y',
        'r','w','g','b','y',
        'r','w','g','b','y'
    },

    State{
        '.',
        'w','g','b','y','r',
        'r','w','g','b','y',
        'r','w','g','b','y',
        'r','w','g','b','y',
        'r','w','g','b','y'
    },

    State{
        '.',
        'w','g','b','y','r',
        'w','g','b','y','r',
        'r','w','g','b','y',
        'r','w','g','b','y',
        'r','w','g','b','y'
    },

    State{
        '.',
        'w','g','b','y','r',
        'w','g','b','y','r',
        'w','g','b','y','r',
        'r','w','g','b','y',
        'r','w','g','b','y'
    },

    State{
        '.',
        'w','g','b','y','r',
        'w','g','b','y','r',
        'w','g','b','y','r',
        'w','g','b','y','r',
        'r','w','g','b','y'
    },

    State{
        '.',
        'w','g','b','y','r',
        'w','g','b','y','r',
        'w','g','b','y','r',
        'w','g','b','y','r',
        'w','g','b','y','r'
    },

    State{
        '.',
        'g','b','y','r','w',
        'w','g','b','y','r',
        'w','g','b','y','r',
        'w','g','b','y','r',
        'w','g','b','y','r'
    },

    State{
        '.',
        'g','b','y','r','w',
        'g','b','y','r','w',
        'w','g','b','y','r',
        'w','g','b','y','r',
        'w','g','b','y','r'
    },

    State{
        '.',
        'g','b','y','r','w',
        'g','b','y','r','w',
        'g','b','y','r','w',
        'w','g','b','y','r',
        'w','g','b','y','r'
    },


    State{
        '.',
        'g','b','y','r','w',
        'g','b','y','r','w',
        'g','b','y','r','w',
        'g','b','y','r','w',
        'g','b','y','r','w'
    }
};

static constexpr int IDS_MAXDEPTH   = 100;
static constexpr int ASTAR_MAXDEPTH = 100;

struct AlgoResult
{
    const char* name;
    int         moves;   // -1 = не найдено
    long long   us;
};

using Clock = std::chrono::steady_clock;

template <typename Fn>
static AlgoResult runAlgorithm(const char* name, Fn&& fn)
{
    AlgoResult r{ name, -1, 0 };
    std::vector<Move> sol;

    auto begin = Clock::now();
    r.moves = fn(sol);
    auto end = Clock::now();

    r.us = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
    return r;
}

static void printRow(const AlgoResult& r)
{
    const std::string steps  = (r.moves < 0) ? "-"       : std::to_string(r.moves);
    const std::string result = (r.moves < 0) ? "no sol." : "solved";

    std::cout << "| "
              << std::left  << std::setw(6)  << r.name
              << " | "
              << std::right << std::setw(8)  << result
              << " | "
              << std::right << std::setw(5)  << steps
              << " | "
              << std::right << std::setw(14) << (std::to_string(r.us) + " ms")
              << " |\n";
}

int main()
{
    std::cout << "IDS maxDepth = " << IDS_MAXDEPTH
              << ", A* maxDepth = "   << ASTAR_MAXDEPTH << "\n\n";

    for (std::size_t s = 0; s < startStates.size(); ++s)
    {
        const State& state = startStates[s];
        BabylonTower tower1(state);
        std::cout << "==============================================================\n";
        std::cout << "State #" << s + 1 << "\n";
        std::cout << tower1 << "\n";
        std::cout << "+--------+----------+-------+----------------+\n";
        std::cout << "| Algo   | Result   | Steps | Time           |\n";
        std::cout << "+--------+----------+-------+----------------+\n";

        {
            BabylonTower tower(state);
            AlgoResult r = runAlgorithm("IDS",
                [&](std::vector<Move>& sol) { return ids(tower, sol, IDS_MAXDEPTH); });
            printRow(r);
        }

        {
            BabylonTower tower(state);
            AlgoResult r = runAlgorithm("A*",
                [&](std::vector<Move>& sol) { return astar(tower, sol, ASTAR_MAXDEPTH); });
            printRow(r);
        }

        std::cout << "+--------+----------+-------+----------------+\n\n";
    }

    return 0;
}
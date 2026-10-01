#include "BabylonTower.h"

constexpr std::array<State, 5> startStates = {
    State{
        '.',
        'w', 'g', 'b', 'y', 'r',
        'r', 'w', 'g', 'b', 'y',
        'r', 'w', 'g', 'b', 'y',
        'r', 'w', 'g', 'b', 'y',
        'r', 'w', 'g', 'b', 'y'
    },

    State{
        '.',
        'r', 'w', 'g', 'b', 'y',
        'r', 'w', 'g', 'b', 'y',
        'r', 'w', 'g', 'b', 'y',
        'r', 'w', 'g', 'b', 'y',
        'r', 'w', 'g', 'b', 'y'
    },

    State{
        '.',
        'w', 'g', 'g', 'r', 'y',
        'w', 'g', 'b', 'b', 'r',
        'r', 'w', 'g', 'g', 'y',
        'y', 'r', 'w', 'b', 'y',
        'r', 'w', 'b', 'y', 'b'
    },

    State{
        '.',
        'g', 'b', 'y', 'r', 'w',
        'b', 'y', 'r', 'w', 'g',
        'y', 'r', 'w', 'g', 'b',
        'r', 'w', 'g', 'b', 'y',
        'w', 'g', 'b', 'y', 'r'
    },

    State{
        '.',
        'b', 'y', 'r', 'w', 'g',
        'y', 'r', 'w', 'g', 'b',
        'r', 'w', 'g', 'b', 'y',
        'g', 'b', 'y', 'r', 'w',
        'w', 'g', 'b', 'y', 'r'
    }
};

int main()
{
    for (const State& state : startStates)
    {
        std::vector<Move> solution;
        auto begin = std::chrono::steady_clock::now();
        int result = ids(BabylonTower(state), solution);
        auto end = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();

        std::cout << "IDS: " << result
                  << ", time: " << ms << " ms\n";
        for (const Move& move : solution)
            std::cout << move << '\n';

        std::cout << '\n';
    }
}
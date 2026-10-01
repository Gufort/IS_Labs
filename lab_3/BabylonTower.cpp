#include "BabylonTower.h"

// Проверка валидности входной последовательности
bool isValid(const State& state)
{
    int count[5] = {};
    int dots = 0;

    for (int i = 1; i <= 25; ++i)
    {
        const char value = state[i];
        if (value == '.')
        {
            dots++;
            continue;
        }


        int color = -1;
        for (int j = 0; j < 5; ++j)
        {
            if (value == colors[j])
            {
                color = j;
                break;
            }
        }
        if (color == -1) return false;
        ++count[color];
    }

    if (dots > 1) return false;

    if (dots == 0)
    {
        for (int i = 0; i < 5; ++i)
        {
            if (count[i] != 5) return false;
        }
    }
    else
    {
        int fours = 0;

        for (int i = 0; i < 5; ++i)
        {
            if (count[i] == 4)++fours;
            else if (count[i] != 5) return false;
        }

        if (fours != 1)
            return false;
    }

    return true;
}

int heuristic(const State& state)
{
    int misplaced = 0;

    for (int r = 0; r < DISCS; ++r)
    {
        for (int c = 0; c < POSITIONS; ++c)
        {
            if (state[idx(r, c)] != GOAL_STATE[idx(r, c)])
            {
                ++misplaced;
            }
        }
    }

    return (misplaced + 4) / 5;
}


// Проверка немедленного обратного хода
bool isImmediateReverse(const Move& prev, const Move& curr)
{
    if (prev.type != curr.type) return false;
    if (prev.index != curr.index) return false;
    if (prev.type == MoveType::SwapTop) return true;

    return prev.direction == -curr.direction;
}

// Основной цикл работы DFS
bool dfsHelper(const BabylonTower& tower, int depth, int limit, std::unordered_set<State, StateHash>& pathVisited,
    std::vector<Move>& path, const std::optional<Move>& previousMove)
{
    if (tower.isSolved()) return true;
    if (depth == limit) return false;
    if (depth + heuristic(tower.state()) > limit)
        return false;

    pathVisited.insert(tower.state());

    for (const Move& move : tower.moves())
    {
        if (previousMove && isImmediateReverse(*previousMove, move))
            continue;

        BabylonTower next = tower.applyMove(move);

        if (pathVisited.count(next.state())) continue;

        path.push_back(move);

        if (dfsHelper(next, depth + 1, limit, pathVisited, path, move))
        {
            pathVisited.erase(next.state());
            return true;
        }

        path.pop_back();
    }

    pathVisited.erase(tower.state());
    return false;
}

// DFS
int dfs(const BabylonTower& start, std::vector<Move>& path, int limit)
{
    path.clear();

    std::unordered_set<State, StateHash> pathVisited;

    if (dfsHelper(start, 0, limit, pathVisited, path, std::nullopt))
        return static_cast<int>(path.size());

    return -1;
}

// IDS
int ids(const BabylonTower& start, std::vector<Move>& solution, int maxDepth)
{
    for (int limit = heuristic(start.state()); limit <= maxDepth; ++limit)
    {
        std::vector<Move> path;
        const int result = dfs(start, path, limit);
        if (result != -1) { solution = std::move(path); return result; }
    }
    return -1;
}




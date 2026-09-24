#include "CrazyPyatnashki.h"

int countOfInversions(const std::string& input, char blank = '0')
{
    std::vector<int> values;

    for (char c : input)
    {
        if (c == blank) continue;
        if ('0' <= c && c <= '9')       values.push_back(c - '0');
        else if ('A' <= c && c <= 'Z')  values.push_back(c - 'A' + 10);
        else throw std::invalid_argument("invalid character");
    }

    int count = 0;
    for (std::size_t i = 0; i < values.size(); ++i)
        for (std::size_t j = i + 1; j < values.size(); ++j)
            if (values[i] > values[j])
                ++count;

    return count;
}

bool isSolvable(const std::string& input)
{
    const int n = static_cast<int>(input.size());
    const int width  = static_cast<int>(std::sqrt(n));
    const int height = n / width;

    const int inversions = countOfInversions(input);

    const int blankPos    = static_cast<int>(input.find('0'));
    const int rowTop0     = blankPos / width;
    const int rowBottom1  = height - rowTop0;

    if (width % 2 == 1)
        return inversions % 2 == 0;

    return (inversions + rowBottom1) % 2 == 1;
}

std::string generateSequence(int width, int steps)
{
    std::string state = TARGET;

    int previousZeroPos = -1;

    for (int i = 0; i < steps; ++i)
    {
        const int zeroPos = static_cast<int>(state.find('0'));

        const int row = zeroPos / width;
        const int col = zeroPos % width;

        std::vector<int> possibleMoves;
        if (row > 0)
            possibleMoves.push_back(zeroPos - width);

        if (row < width - 1)
            possibleMoves.push_back(zeroPos + width);

        if (col > 0)
            possibleMoves.push_back(zeroPos - 1);

        if (col < width - 1)
            possibleMoves.push_back(zeroPos + 1);

        // Не идём сразу обратно
        if (possibleMoves.size() > 1)
        {
            possibleMoves.erase(
                std::remove(
                    possibleMoves.begin(),
                    possibleMoves.end(),
                    previousZeroPos
                ),
                possibleMoves.end()
            );
        }

        int newZeroPos = possibleMoves[rand() % possibleMoves.size()];

        previousZeroPos = zeroPos;

        std::swap(state[zeroPos], state[newZeroPos]);
    }

    return state;
}

// Найдем все состаяния, куда 0 может перейти за один ход
std::vector<std::string> getNeighbours(const std::string& input)
{
    std::vector<std::string> neighbours;
    int width  = static_cast<int>(std::sqrt(input.size()));

    const int zeroPos = static_cast<int>(input.find('0'));

    const int row = zeroPos / width;
    const int col = zeroPos % width;

    auto addMove = [&](int newRow, int newCol)
    {
        if (newRow < 0 || newRow >= width || newCol < 0 || newCol >= width)
            return;
        const int newPos = newRow * width + newCol;

        auto next = input;
        std::swap(next[zeroPos], next[newPos]);

        neighbours.push_back(next);
    };

    addMove(row - 1, col); addMove(row + 1, col); addMove(row, col - 1); addMove(row, col + 1);
    return neighbours;
}


// Решения с помощью различных алгоритмов
// BFS
int bfs(const std::string& input)
{
    if (!isSolvable(input)) throw std::invalid_argument("not solvable: " + input);

    std::queue<std::string> queue;
    std::unordered_map<std::string, int> distance;

    queue.push(input);
    distance[input] = 0;

    while (!queue.empty())
    {
        std::string current = queue.front();
        queue.pop();

        if (current == TARGET)
            return distance[current];

        for (const std::string& neighbour : getNeighbours(current))
        {
            if (distance.contains(neighbour))
                continue;

            distance[neighbour] = distance[current] + 1;
            queue.push(neighbour);
        }
    }

    return -1;
}

// DFS
int dfs(const std::string& input)
{
    if (!isSolvable(input)) throw std::invalid_argument("not solvable: " + input);

    for (int limit = 0; ; ++limit)
    {
        std::unordered_set<std::string> visited;
        if (helpDfs(input, 0, limit, visited)) return limit;
    }
}

bool helpDfs(const std::string& current, int depth, int limit, std::unordered_set<std::string>& visited)
{
    if (current == TARGET) return true;
    if (depth == limit) return false;

    visited.insert(current);

    for (const std::string& neighbour : getNeighbours(current))
    {
        if (visited.contains(neighbour)) continue;
        if (helpDfs(neighbour, depth + 1, limit, visited)) return true;
    }

    visited.erase(current);
    return false;
}





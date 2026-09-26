#include "CrazyPyatnashki.h"

int  newBlankTable[BOARD_SIZE][4];
bool isMovesInit = false;

void initializeMoves()
{
    for (int i = 0; i < BOARD_WIDTH; i++)
    {
        for (int j = 0; j < BOARD_WIDTH; j++)
        {
            int pos = i * BOARD_WIDTH + j;
            newBlankTable[pos][0] = (i > 0) ? pos - BOARD_WIDTH : -1;
            newBlankTable[pos][1] = (i < BOARD_WIDTH - 1) ? pos + BOARD_WIDTH : -1;
            newBlankTable[pos][2] = (j > 0) ? pos - 1 : -1;
            newBlankTable[pos][3] = (j < BOARD_WIDTH - 1) ? pos + 1 : -1;
        }
    }
    isMovesInit = true;
}

int getNibble(u64 state, int pos) {
    return (state >> (4 * (15 - pos))) & 0xF;
}

u64 setNibble(u64 state, int pos, int value)
{
    int shift = 4 * (15 - pos);
    auto mask = 0xFULL << shift;
    return (state & ~mask) | (u64(value) << shift);
}

int findBlankPosition(u64 state)
{
    for (int i = 0; i < BOARD_SIZE; i++)
        if (getNibble(state, i) == 0) return i;
    return -1;
}

// Двигаем нолик на новую позицию
u64 shiftBlankPosition(u64 state, int blankPosition, int newBlankPosition)
{
    int tile = getNibble(state, newBlankPosition);
    state = setNibble(state, blankPosition, tile);
    state = setNibble(state, newBlankPosition, 0);
    return state;
}

u64 parseState(const std::string& input)
{
    if (static_cast<int>(input.size()) != BOARD_SIZE)
        throw std::runtime_error("Wrong state size, not equal " + std::to_string(BOARD_SIZE));

    u64 state = 0;
    for (char c : input)
    {
        int value;
        if ('0' <= c && c <= '9') value = c - '0';
        else if ('A' <= c && c <= 'F') value = c - 'A' + 10;
        else throw std::runtime_error(std::string("invalid character ") + c);
        state = (state << 4) | (static_cast<u64>(value));
    }

    return state;
}

std::string stateToString(uint64_t state)
{
    std::string result;
    result.reserve(BOARD_SIZE);
    for (int pos = 0; pos < BOARD_SIZE; ++pos) {
        const int value = getNibble(state, pos);
        result += (value < 10) ? static_cast<char>('0' + value)
                      : static_cast<char>('A' + value - 10);
    }
    return result;
}

// Проверка на разрешимость заданной последовательности
int countOfInversions(uint64_t state)
{
    std::vector<int> values;
    values.reserve(BOARD_SIZE - 1);
    for (int pos = 0; pos < BOARD_SIZE; ++pos)
    {
        const int value = getNibble(state, pos);
        if (value == 0) continue;
        values.push_back(value);
    }

    int count = 0;
    for (int i = 0; i < values.size(); ++i)
    {
        for (int j = i + 1; j < values.size(); ++j)
        {
            if (values[i] > values[j]) ++count;
        }
    }

    return count;
}

bool isSolvable(u64 state)
{
    const int inversions = countOfInversions(state);
    const int blankPos = findBlankPosition(state);
    const int rowTop0 = blankPos / BOARD_WIDTH;
    const int rowBottom1 = BOARD_WIDTH - rowTop0;

    return (inversions + rowBottom1) % 2 == 1;
}

std::vector<u64> getNeighbours(u64 state)
{
    if (!isMovesInit) initializeMoves();
    std::vector<u64> neighbours;
    neighbours.reserve(BOARD_WIDTH);

    const int blankPos = findBlankPosition(state);
    for (int i = 0; i < BOARD_WIDTH; ++i)
    {
        int neighbour = newBlankTable[blankPos][i];
        if (neighbour < 0) continue;
        neighbours.push_back(shiftBlankPosition(state, blankPos, neighbour));
    }

    return neighbours;
}

// Рассмотрим шизоалгоритмы решения сей задачи
// BFS
int bfs(u64 state)
{
    if (!isSolvable(state)) throw std::runtime_error("not solvable " + stateToString(state));
    if (state == TARGET) return 0;

    custom_map distance(23);
    std::queue<u64> queue;

    distance.insert_or_assign(state, 0);
    queue.push(state);

    while (!queue.empty())
    {
        u64 current = queue.front();
        queue.pop();
        auto d = *distance.find(current);

        for (auto neighbour : getNeighbours(current))
        {
            if (distance.find(neighbour)) continue;
            if (neighbour == TARGET) return d + 1;
            distance.insert_or_assign(neighbour, d + 1);
            queue.push(neighbour);
        }
    }
    return -1;
}

// DFS
int dfs(u64 state, int maxDepth)
{
    if (!isSolvable(state))
        throw std::runtime_error("not solvable " + stateToString(state));

    custom_set visited(22);
    return helpDfs(state, 0, maxDepth, visited);
}

int helpDfs(u64 current, int depth, int limit, custom_set& visited)
{
    if (current == TARGET)   return depth;
    if (depth >= limit)   return -1;
    if (!visited.insert(current)) return -1;

    for (u64 neighbour : getNeighbours(current)) {
        const int result = helpDfs(neighbour, depth + 1, limit, visited);
        if (result != -1) return result;
    }
    return -1;
}

// Конфликты по строкам и столбцам
static int goalRow[16];   // goalRow[t] = строка, где должна стоять плитка t
static int goalCol[16];   // goalCol[t] = столбец, где должна стоять плитка t

static int linearConflict(u64 state)
{
    int tiles[16];
    for (int i = 0; i < 16; ++i)
        tiles[i] = getNibble(state, i);

    int conflicts = 0;

    // Конфликты по строкам
    for (int r = 0; r < BOARD_WIDTH; ++r) {
        for (int c1 = 0; c1 < BOARD_WIDTH; ++c1) {
            const int t1 = tiles[r * BOARD_WIDTH + c1];
            if (t1 == 0) continue;
            if (goalRow[t1] != r) continue;             // t1 не в своей строке
            const int gc1 = goalCol[t1];

            for (int c2 = c1 + 1; c2 < BOARD_WIDTH; ++c2) {
                const int t2 = tiles[r * BOARD_WIDTH + c2];
                if (t2 == 0) continue;
                if (goalRow[t2] != r) continue;         // t2 не в своей строке
                const int gc2 = goalCol[t2];

                // t1 левее t2 на поле, но в цели t1 должна быть правее
                if (gc1 > gc2) conflicts += 2;
            }
        }
    }

    // Конфликты по столбцам — аналогично
    for (int c = 0; c < BOARD_WIDTH; ++c) {
        for (int r1 = 0; r1 < BOARD_WIDTH; ++r1) {
            const int t1 = tiles[r1 * BOARD_WIDTH + c];
            if (t1 == 0) continue;
            if (goalCol[t1] != c) continue;             // t1 не в своём столбце
            const int gr1 = goalRow[t1];

            for (int r2 = r1 + 1; r2 < BOARD_WIDTH; ++r2) {
                const int t2 = tiles[r2 * BOARD_WIDTH + c];
                if (t2 == 0) continue;
                if (goalCol[t2] != c) continue;         // t2 не в своём столбце
                const int gr2 = goalRow[t2];

                // t1 выше t2, но в цели t1 должна быть ниже
                if (gr1 > gr2) conflicts += 2;
            }
        }
    }

    return conflicts;
}

// Манхэттенское расстояние
static int  manhattan[16][16];
static bool manhattanInit = false;

void initManhattan()
{
    for (int t = 1; t <= 15; ++t) {
        const int target = t - 1;
        goalRow[t] = (t - 1) / BOARD_WIDTH;
        goalCol[t] = (t - 1) % BOARD_WIDTH;
        const int tr = target / BOARD_WIDTH;
        const int tc = target % BOARD_WIDTH;
        for (int pos = 0; pos < BOARD_SIZE; ++pos) {
            const int r = pos / BOARD_WIDTH;
            const int c = pos % BOARD_WIDTH;
            manhattan[t][pos] = std::abs(r - tr) + std::abs(c - tc);
        }
    }
    manhattanInit = true;
}

int heuristic(u64 state)
{
    int sum = 0;
    for (int pos = 0; pos < BOARD_SIZE; ++pos) {
        const int t = getNibble(state, pos);
        if (t) sum += manhattan[t][pos];
    }
    return sum + linearConflict(state);
}

// IDS
int ids(u64 state)
{
    if (!isSolvable(state)) throw std::runtime_error("not solvable " + stateToString(state));
    if (!manhattanInit) initManhattan();
    if (state == TARGET) return 0;

    custom_set visited(20);

    for (int limit = heuristic(state); ; ++limit) {
        visited.clear();
        if (helpIds(state, 0, limit, visited)) return limit;
    }
}

bool helpIds(u64 current, int depth, int limit, custom_set& visited)
{
    if (current == TARGET) return true;
    if (depth == limit) return false;
    if (depth + heuristic(current) > limit) return false;

    visited.insert(current);

    for (u64 neighbour : getNeighbours(current)) {
        if (visited.contains(neighbour)) continue;
        if (helpIds(neighbour, depth + 1, limit, visited)) return true;
    }

    visited.erase(current);
    return false;
}

// A*
int astar(u64 state)
{
    if (!isSolvable(state)) throw std::runtime_error("not solvable " + stateToString(state));
    if (state == TARGET) return 0;
    if (!manhattanInit) initManhattan();

    std::priority_queue<std::pair<int, u64>, std::vector<std::pair<int, u64>>, std::greater<>> open; // f, state (f = g + h), храним в виде вектора
    custom_map g_score(23); // лучшая длина от старта до текущего узла
    custom_set closed(23);

    g_score.insert_or_assign(state, 0);
    open.push({heuristic(state), state});

    while (!open.empty())
    {
        auto [f, current] = open.top();
        open.pop();

        if (closed.contains(current)) continue;
        closed.insert(current);

        const int g = *g_score.find(current);
        if (current == TARGET) return g;

        for (auto neighbour : getNeighbours(current))
        {
            if (closed.contains(neighbour)) continue;

            int ng = 1 + g;
            const int* old = g_score.find(neighbour); // лучшее положение для соседа
            if (old && *old <= ng) continue; // новый путь хуже, идем дальше

            g_score.insert_or_assign(neighbour, ng);
            open.push({ ng + heuristic(neighbour), neighbour });
        }
    }

    return -1;
}

// IDA*
int idaStar(u64 state)
{
    int inf = 1000000000;
    if (!isSolvable(state))
        throw std::runtime_error("not solvable " + stateToString(state));
    if (state == TARGET) return 0;
    if (!manhattanInit) initManhattan();

    int threshold = heuristic(state);
    custom_set visited(22);

    while (true)
    {
        visited.clear();
        int nextThreshold = inf; // min f, что превысило threshold
        auto res = helpIdaStar(state, 0, threshold, visited, nextThreshold);

        if (res >= 0) return res;
        if (nextThreshold >= inf) return -1;

        threshold = nextThreshold;
    }
}

int helpIdaStar(u64 current, int g, int threshold, custom_set& visited, int& nextThreshold)
{
    const int f = g + heuristic(current);
    if (f > threshold) {
        if (f < nextThreshold) nextThreshold = f;
        return -1;
    }

    if (current == TARGET) return g;

    visited.insert(current);

    for (auto neighbour : getNeighbours(current))
    {
        if (visited.contains(neighbour)) continue;

        const int res = helpIdaStar(neighbour, g + 1, threshold, visited, nextThreshold);

        if (res >= 0) return res;
    }

    visited.erase(current);
    return -1;
}






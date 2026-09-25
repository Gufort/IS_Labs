#include "CrazyPyatnashki.h"

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
        throw std::runtime_error("Wrong state size, not equal " + BOARD_SIZE);

    u64 state = 0;
    for (char c : input)
    {
        int value;
        if ('0' <= c && c <= '9') value = c - '0';
        else if ('A' <= c && c <= 'F') value = c - 'A' + 10;
        else throw std::runtime_error("invalid character " + c);
        state = (state << 4) | (static_cast<u64>(value));
    }

    return state;
}

inline std::string stateToString(uint64_t state)
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
inline int countOfInversions(uint64_t state)
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

inline bool isSolvable(u64 state)
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
        neighbours.push_back(neighbour);
    }

    return neighbours;
}

// Рассмотрим шизоалгоритмы решения сей задачи



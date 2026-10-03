#pragma once
#include <cstdint>
#include <array>
#include <iosfwd>
#include <iostream>
#include <vector>
#include <chrono>
#include <optional>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <cmath>
#include <cstdlib>

using State = std::array<char, 26>;

constexpr int DISCS = 5;
constexpr int POSITIONS = 5;
constexpr int TOP = 0;
constexpr int TOP_COL = 0;
constexpr char colors[] = { 'r', 'w', 'g', 'b', 'y' };
constexpr State GOAL_STATE = {
    '.',
    'r','w','g','b','y',
    'r','w','g','b','y',
    'r','w','g','b','y',
    'r','w','g','b','y',
    'r','w','g','b','y'
};

struct StateHash
{
    std::size_t operator()(const State& s) const noexcept
    {
        std::size_t h = 1469598103934665603ull;

        for (char c : s)
        {
            h ^= static_cast<unsigned char>(c);
            h *= 1099511628211ull;
        }

        return h;
    }
};

constexpr int idx(int row, int column) { return 1 + row * POSITIONS + column; }

// Движения
enum class MoveType
{
    Row,    // Поворот диска
    Column, // Движение по вертикали вдоль колонки
    SwapTop // Свап с пустой верхней ячейкой
};

struct Move
{
    MoveType type;
    uint8_t index = 0;
    int8_t direction = 0;

    friend bool operator==(const Move&, const Move&) = default;
};

inline std::ostream& operator<<(std::ostream& os, const Move& move)
{
    switch (move.type)
    {
    case MoveType::Row:
        os << "R" << static_cast<int>(move.index);
        os << (move.direction > 0 ? " ->" : " <-");
        break;

    case MoveType::Column:
        os << "C" << static_cast<int>(move.index);
        os << (move.direction > 0 ? " down" : " up");
        break;

    case MoveType::SwapTop:
        os << "SwapTop";
        break;
    }

    return os;
}

class BabylonTower
{
public:
    explicit BabylonTower(const State& state): _state(state) {}
    const State& state() const { return _state; }
    bool isSolved() const { return _state == GOAL_STATE; }

    // Все возможные ходы. Возвращает количество ходов.
    int moves(Move* out) const
    {
        int n = 0;

        for (int r = 0; r < DISCS; ++r)
        {
            if (rowUniform(r)) continue;
            out[n++] = { MoveType::Row, static_cast<uint8_t>(r), +1 };
            out[n++] = { MoveType::Row, static_cast<uint8_t>(r), -1 };
        }

        for (int c = 0; c < POSITIONS; ++c)
        {
            if (colUniform(c)) continue;
            out[n++] = { MoveType::Column, static_cast<uint8_t>(c), -1 };
            out[n++] = { MoveType::Column, static_cast<uint8_t>(c), +1 };
        }

        const bool topEmpty = _state[TOP] == '.';
        const bool diskEmpty = _state[idx(0, TOP_COL)] == '.';

        if (topEmpty != diskEmpty)
            out[n++] = { MoveType::SwapTop, 0, 0 };

        return n;
    }

    // Применение хода
    void applyMove(const Move& move)
    {
        switch (move.type)
        {
            case MoveType::Row:     rotateRow(_state, move.index, move.direction); break;
            case MoveType::Column:  rotateColumn(_state, move.index, move.direction); break;
            case MoveType::SwapTop: std::swap(_state[TOP], _state[idx(0, TOP_COL)]); break;
        }
    }

    // Откат хода
    void undoMove(const Move& move)
    {
        Move inv = move;
        if (move.type == MoveType::Row || move.type == MoveType::Column)
            inv.direction = static_cast<int8_t>(-move.direction);
        applyMove(inv);
    }

    friend std::ostream& operator<<(std::ostream& os, const BabylonTower& tower)
    {
        os << "        ";

        for (int c = 0; c < POSITIONS; ++c)
        {
            if (c == TOP_COL)
                os << "[ " << tower._state[TOP] << " ]";
            else
                os << "     ";
        }

        os << '\n';
        os << "    +---+---+---+---+---+\n";

        for (int r = 0; r < DISCS; ++r)
        {
            os << "    |";

            for (int c = 0; c < POSITIONS; ++c)
            {
                char value = tower._state[idx(r, c)];

                if (value == '.')
                    os << " . |";
                else
                    os << ' ' << value << " |";
            }

            os << '\n';
            os << "    +---+---+---+---+---+\n";
        }

        return os;
    }

private:
    State _state;

    bool rowUniform(int row) const
    {
        const char first = _state[idx(row, 0)];
        for (int col = 1; col < POSITIONS; ++col)
        {
            if (_state[idx(row, col)] != first)
                return false;
        }
        return true;
    }

    bool colUniform(int col) const
    {
        const char first = _state[idx(0, col)];
        for (int row = 1; row < DISCS; ++row)
        {
            if (_state[idx(row, col)] != first)
                return false;
        }
        return true;
    }

    static void rotateRow(State& state, int row, int direction)
    {
        if (direction > 0)
        {
            const char first = state[idx(row, 0)];
            for (int column = 0; column < POSITIONS - 1; ++column)
                state[idx(row, column)] = state[idx(row, column + 1)];
            state[idx(row, POSITIONS - 1)] = first;
        }
        else
        {
            const char last = state[idx(row, POSITIONS - 1)];
            for (int column = POSITIONS - 1; column > 0; --column)
                state[idx(row, column)] = state[idx(row, column - 1)];
            state[idx(row, 0)] = last;
        }
    }

    // Вертикальный сдвиг колонки (исправлен выход за границы при direction < 0)
    static void rotateColumn(State& state, int column, int direction)
    {
        if (direction > 0)
        {
            const char first = state[idx(0, column)];
            for (int row = 0; row < DISCS - 1; ++row)
                state[idx(row, column)] = state[idx(row + 1, column)];
            state[idx(DISCS - 1, column)] = first;
        }
        else
        {
            const char last = state[idx(DISCS - 1, column)];
            for (int row = DISCS - 1; row > 0; --row)
                state[idx(row, column)] = state[idx(row - 1, column)];
            state[idx(0, column)] = last;
        }
    }
};

static bool dfsHelper(BabylonTower& tower, int depth, int limit,
    std::unordered_map<State, int, StateHash>& best,
    std::vector<Move>& path,
    const std::optional<Move>& prev, int h);
int dfs(BabylonTower& tower, std::vector<Move>& solution, int limit);
int ids(BabylonTower& tower, std::vector<Move>& solution, int maxDepth);
int astar(BabylonTower& start, std::vector<Move>& solution, int maxDepth);
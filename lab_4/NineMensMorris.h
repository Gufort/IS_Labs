#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <optional>

// Структура игрока
enum class Player: uint8_t
{
    Player0 = 0,
    Player1 = 1
};

constexpr Player opponent(Player player)
{
    return player == Player::Player0 ? Player::Player1 : Player::Player0;
}

// Клетка доски
enum class Cell: uint8_t
{
    Empty, Player0, Player1
};

// Ход
struct Move {
    int from = -1; // откуда
    int to = -1; // куда
    int capture = -1; // позиция вражеской пешки, если мы ее снимаем с доски
};

class GameBoard
{
public:
    static constexpr int Size = 24;
    static constexpr int MaxNeighbours = 4;

    using Position = int;
    using Mill = std::array<Position, 3>; // под мельницей понимается три подряд стоящие фишки одного цвета
    using NeighbourList = std::array<Position, MaxNeighbours>;

    static const std::array<Mill, 16>& getMills(){ return _mills; }

    static bool isValidPosition(const Position pos)
    {
        return pos >= 0 && pos < Size;
    }

    static const NeighbourList& neighbours(const Position position)
    {
        return _neighbours[position];
    }

    // Проверяет, соединены ли две позиции ребром графа
    static bool areAdjacent(const Position first, const Position second)
    {
        if (!isValidPosition(first) || !isValidPosition(second))
            return false;

        const auto& neighboursOfFirst = _neighbours[first];

        for (auto neighbour : neighboursOfFirst)
            if (neighbour == second)
                return true;

        return false;
    }

    // Проверяет, входит ли позиция в конкретную мельницу
    static bool containsPosition(const Mill& mill, const Position position)
    {
        for (auto millPosition : mill)
            if (millPosition == position)
                return true;

        return false;
    }

private:
    /*
        0----------------1------------------2
        |                                   |
        |     8----------9------------10    |
        |     |                       |     |
        |     |    16----17-----18    |     |
        |     |    |             |    |     |
        |     |    |             |    |     |
        7-----15---23            19---11----3
        |     |    |             |    |     |
        |     |    |             |    |     |
        |     |    22----21-----20    |     |
        |     |                       |     |
        |     14---------13-----------12    |
        |                                   |
        6----------------5------------------4
        Для понимания индексации в графе выше представлен рисунок доски.
        У каждой позиции есть максимум 4 соседа, в случае его отсутствия - ставим -1
    */
    static constexpr std::array<NeighbourList, Size> _neighbours = {{
        {{1, 7, -1, -1}},       // 0
        {{0, 2, 9, -1}},        // 1
        {{1, 3, -1, -1}},       // 2
        {{2, 4, 11, -1}},       // 3
        {{3, 5, -1, -1}},       // 4
        {{4, 6, 13, -1}},       // 5
        {{5, 7, -1, -1}},       // 6
        {{6, 0, 15, -1}},       // 7

        {{9, 15, -1, -1}},      // 8
        {{8, 10, 1, 17}},       // 9
        {{9, 11, -1, -1}},      // 10
        {{10, 12, 3, 19}},      // 11
        {{11, 13, -1, -1}},     // 12
        {{12, 14, 5, 21}},      // 13
        {{13, 15, -1, -1}},     // 14
        {{14, 8, 7, 23}},       // 15

        {{17, 23, -1, -1}},     // 16
        {{16, 18, 9, -1}},      // 17
        {{17, 19, -1, -1}},     // 18
        {{18, 20, 11, -1}},      // 19
        {{19, 21, -1, -1}},     // 20
        {{20, 22, 13, -1}},     // 21
        {{21, 23, -1, -1}},     // 22
        {{22, 16, 15, -1}}       // 23
    }};

    // Все возможные мельницы
    static constexpr std::array<Mill, 16> _mills = {{
        // Внешний квадрат
        {{0, 1, 2}},
        {{2, 3, 4}},
        {{4, 5, 6}},
        {{6, 7, 0}},

        // Средний квадрат
        {{8, 9, 10}},
        {{10, 11, 12}},
        {{12, 13, 14}},
        {{14, 15, 8}},

        // Внутренний квадрат
        {{16, 17, 18}},
        {{18, 19, 20}},
        {{20, 21, 22}},
        {{22, 23, 16}},

        // Соединения между квадратами
        {{1, 9, 17}},
        {{3, 11, 19}},
        {{5, 13, 21}},
        {{7, 15, 23}}
    }};
};


struct GameState
{
    std::array<Cell, GameBoard::Size> board;
    Player currentPlayer;
    std::array<int, 2> piecesInHand; // Кол-во фишек у каждого из игроков
};


class NineMensMorris
{
public:
    static constexpr int PiecesPerPlayer = 9;

    NineMensMorris() { reset(); }

    // Состояние игры
    const GameState& getGameState() const
    {
        return _gameState;
    }

    Player currentPlayer() const
    {
        return _gameState.currentPlayer;
    }

    Cell cell(GameBoard::Position position) const
    {
        return _gameState.board[position];
    }

    int piecesInHand(Player player) const
    {
        return _gameState.piecesInHand[playerIndex(player)];
    }

    int piecesOnBoard(Player player) const
    {
        int count = 0;

        auto playerCell = cellForPlayer(player);

        for (auto cell : _gameState.board)
            if (cell == playerCell)
                ++count;

        return count;
    }

    // Ходы
    std::vector<Move> legalMoves() const
    {
        std::vector<Move> result;
        Player player = currentPlayer();

        // Выставляем фишки
        if (piecesInHand(player) > 0)
        {
            for (int position = 0; position < GameBoard::Size; ++position)
            {
                if (getGameState().board[position] != Cell::Empty)
                    continue;

                Move move;
                move.from = -1;
                move.to = position;
                move.capture = -1;

                // Проверяем, образует ли установка мельницу.
                if (formsMill(move, player))
                    addCapturesMoves(result, move, player);
                else
                    result.push_back(move);
            }

            return result;
        }

        // Двигаем поставленные фишки
        int pieces = piecesOnBoard(player);

        if (pieces < 3)
            return result;

        for (int from = 0; from < GameBoard::Size; ++from)
        {
            if (_gameState.board[from] != cellForPlayer(player))
                continue;

            // В случае, если у игрока всего 3 шашки - он может прыгать в любую ячейку
            if (pieces == 3)
            {
                for (int to = 0; to < GameBoard::Size; ++to)
                {
                    if (_gameState.board[to] != Cell::Empty)
                        continue;

                    Move move;
                    move.from = from;
                    move.to = to;
                    move.capture = -1;

                    if (formsMill(move, player))
                        addCapturesMoves(result, move, player);
                    else
                        result.push_back(move);
                }

                continue;
            }

            // Обычное движение - только на соседние клетки
            const auto& neighbours = GameBoard::neighbours(from);

            for (int to : neighbours)
            {
                if (to == -1)
                    continue;

                if (_gameState.board[to] != Cell::Empty)
                    continue;

                Move move;
                move.from = from;
                move.to = to;
                move.capture = -1;

                if (formsMill(move, player))
                    addCapturesMoves(result, move, player);
                else
                    result.push_back(move);
            }
        }

        return result;
    }

    bool isLegalMove(const Move& move) const
    {
        const auto moves = legalMoves();

        for (const Move& legalMove : moves)
        {
            if (legalMove.from == move.from &&
                legalMove.to == move.to &&
                legalMove.capture == move.capture)
            {
                return true;
            }
        }

        return false;
    }

    bool makeMove(const Move& move)
    {
        if (!isLegalMove(move))
            return false;

        _history.emplace_back(move, _gameState);

        Player player = currentPlayer();
        Cell playerCell = cellForPlayer(player);

        // Установка новой фишки
        if (move.from == -1)
        {
            _gameState.board[move.to] = playerCell;
            --_gameState.piecesInHand[playerIndex(player)];
        }

        // Передвижение существующей шашки
        else
        {
            _gameState.board[move.from] = Cell::Empty;
            _gameState.board[move.to] = playerCell;
        }

        // Снятие шашки противника
        if (move.capture != -1)
        {
            _gameState.board[move.capture] = Cell::Empty;
        }

        _gameState.currentPlayer = opponent(player);

        return true;
    }

    // Мельницы
    bool formsMill(const Move& move, Player player) const
    {
        Cell playerCell = cellForPlayer(player);

        GameState tempState = _gameState;

        if (move.from == -1)
            tempState.board[move.to] = playerCell;
        else
        {
            tempState.board[move.from] = Cell::Empty;
            tempState.board[move.to] = playerCell;
        }

        for (const auto& mill : GameBoard::getMills())
        {
            if (!GameBoard::containsPosition(mill, move.to))
                continue;

            bool completed = true;

            for (auto position : mill)
            {
                if (tempState.board[position] != playerCell)
                {
                    completed = false;
                    break;
                }
            }

            if (completed)
                return true;
        }

        return false;
    }

    bool isInMill(GameBoard::Position position, Player player) const
    {
        Cell playerCell = cellForPlayer(player);

        for (const auto& mill : GameBoard::getMills())
        {
            if (!GameBoard::containsPosition(mill, position))
                continue;

            bool complete = true;

            for (GameBoard::Position millPosition : mill)
            {
                if (_gameState.board[millPosition] != playerCell)
                {
                    complete = false;
                    break;
                }
            }

            if (complete)
                return true;
        }

        return false;
    }

    std::vector<int> captureCandidates(Player player) const
    {
        std::vector<int> candidates;

        Cell playerCell = cellForPlayer(player);

        // Сначала ищем шашки, которые НЕ находятся в мельнице
        for (int position = 0; position < GameBoard::Size; ++position)
        {
            if (getGameState().board[position] != playerCell)
                continue;

            if (!isInMill(position, player))
                candidates.push_back(position);
        }

        // Если все шашки находятся в мельницах - разрешается снять любую
        if (candidates.empty())
        {
            for (int position = 0; position < GameBoard::Size; ++position)
            {
                if (_gameState.board[position] == playerCell)
                    candidates.push_back(position);
            }
        }

        return candidates;
    }

    // Конец игры
    bool isGameOver() const
    {
        Player player = currentPlayer();

        if (piecesOnBoard(player) < 3 && piecesInHand(player) == 0)
            return true;

        if (piecesInHand(player) > 0)
            return false;

        return legalMoves().empty();
    }

    std::optional<Player> winner() const
    {
        if (!isGameOver())
            return std::nullopt;

        return opponent(currentPlayer());
    }

    // История
    const std::vector<std::pair<Move, GameState>>& history() const
    {
        return _history;
    }

    bool undo()
    {
        if (_history.empty())
            return false;

        _gameState = _history.back().second;
        _history.pop_back();

        return true;
    }

    void reset()
    {
        _gameState.board.fill(Cell::Empty);

        _gameState.currentPlayer = Player::Player0;

        _gameState.piecesInHand[0] = PiecesPerPlayer;
        _gameState.piecesInHand[1] = PiecesPerPlayer;

        _history.clear();
    }

private:
    static int playerIndex(Player player)
    {
        return static_cast<int>(player);
    }

    static Cell cellForPlayer(Player player)
    {
        return player == Player::Player0
            ? Cell::Player0
            : Cell::Player1;
    }

    void addCapturesMoves (std::vector<Move>& moves, const Move& baseMove, Player player) const
    {
        Player opponentForPlayer = opponent(player);

        for (auto capture : captureCandidates(opponentForPlayer))
        {
            Move move = baseMove;
            move.capture = capture;

            moves.push_back(move);
        }
    }

private:
    GameBoard _board;
    GameState _gameState;
    std::vector<std::pair<Move, GameState>> _history;
};
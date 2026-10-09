#pragma once

#include "NineMensMorris.h"
#include <algorithm>
#include <limits.h>
#include <limits>
#include <vector>

class GameBot
{
public:
    explicit GameBot(int searchDepth = 4) : _searchDepth(searchDepth) {};

    std::optional<Move> findBestMove(NineMensMorris& game)
    {
        const auto moves = game.legalMoves();

        if (moves.empty())
            return std::nullopt;

        std::optional<Move> bestMove;
        int bestScore = -INT_MAX;
        int alpha = -INT_MAX;
        int beta = INT_MAX;

        for (const auto& move : moves)
        {
            if (!game.makeMove(move))
                continue;

            const int score = -negamax(game, _searchDepth - 1, -beta, -alpha, 1);

            game.undo();

            if (score > bestScore)
            {
                bestScore = score;
                bestMove = move;
            }

            alpha = std::max(alpha, score);
        }

        return bestMove;
    }

private:

    int evaluate(const NineMensMorris& game) const
    {
        const GameState& state = game.getGameState();

        const Player player = state.currentPlayer;
        const Player enemy = opponent(player);

        int score = 0;

        // Материальное преимущество.
        score += 100 * (
            game.piecesOnBoard(player) -
            game.piecesOnBoard(enemy)
        );

        // Фишки, оставшиеся в руке.
        score += 80 * (
            game.piecesInHand(player) -
            game.piecesInHand(enemy)
        );

        // Открытые мельницы, развилки и двойные угрозы.
        score += threatScore(game, player);
        score -= threatScore(game, enemy);

        // Заблокированные фишки.
        score -= 6 * (
            blockedPieces(game, player) -
            blockedPieces(game, enemy)
        );

        // Потенциал немедленного создания мельниц.
        score += 30 * countMillCreatingTargets(
            const_cast<NineMensMorris&>(game)
        );

        // Немедленные угрозы создания мельниц противником.
        score -= 35 * countImmediateMillTargets(state, enemy);

        return score;
    }

    int negamax(NineMensMorris& game, int depth, int alpha, int beta, int ply)
    {
        if (game.isGameOver())
            return -WinScore + ply;

        if (depth <= 0)
            return evaluate(game);

        auto moves = game.legalMoves();

        int bestScore = -INT_MAX;

        for (const auto& move : moves)
        {
            if (!game.makeMove(move))
                continue;

            // Рекурсивно оцениваем ответ противника.
            const int score = -negamax(game,depth - 1,-beta,-alpha,ply + 1);

            // Отменяем ход, чтобы исследовать следующую ветвь из той же позиции
            game.undo();

            bestScore = std::max(bestScore, score);
            alpha = std::max(alpha, score);

            if (alpha >= beta)
                break;
        }
        return bestScore;
    }

    int threatScore(const NineMensMorris& game, Player player) const
    {
        const auto& state = game.getGameState();

        const Cell ownCell = cellForPlayer(player);
        const Cell enemyCell = cellForPlayer(opponent(player));

        // Для каждой точки считаем количество мельниц, которые можно закрыть размещением фишки в этой точке
        std::array<int, GameBoard::Size> targetCounts{};

        int openMills = 0;

        for (const auto& mill : GameBoard::getMills())
        {
            int ownPieces = 0;
            int enemyPieces = 0;
            int emptyCells = 0;
            int emptyPosition = -1;

            for (const int position : mill)
            {
                const Cell cell = state.board[position];

                if (cell == ownCell)
                    ++ownPieces;
                else if (cell == enemyCell)
                    ++enemyPieces;
                else
                {
                    ++emptyCells;
                    emptyPosition = position;
                }
            }

            // Две наши фишки и одна пустая точка
            if (ownPieces == 2 && enemyPieces == 0 && emptyCells == 1)
            {
                ++openMills;
                ++targetCounts[emptyPosition];
            }
        }

        int distinctTargets = 0;
        int doubleMillTargets = 0;
        for (const int count : targetCounts)
        {
            if (count > 0)
                ++distinctTargets;

            // Одна точка закрывает сразу несколько мельниц.
            if (count >= 2)
                ++doubleMillTargets;
        }

        int score = 0;

        // Каждая открытая мельница представляет угрозу.
        score += 10 * openMills;

        // Две разные точки завершения — развилка
        if (distinctTargets >= 2)
            score += 25;

        // Одна точка завершает несколько мельниц
        score += 15 * doubleMillTargets;

        return score;
    }

    int blockedPieces(const NineMensMorris& game, Player player) const
    {
        if (game.piecesInHand(player) > 0)
            return 0;

        if (game.piecesOnBoard(player) <= 3)
            return 0;

        const auto& state = game.getGameState();

        const Cell ownCell = cellForPlayer(player);
        const Cell enemyCell = cellForPlayer(opponent(player));

        int blocked = 0;

        for (int position = 0; position < GameBoard::Size; ++position)
        {
            if (state.board[position] != ownCell)
                continue;

            bool hasEmptyNeighbour = false;

            for (const int neighbour : GameBoard::neighbours(position))
            {
                if (neighbour == -1)
                    continue;

                const Cell cell = state.board[neighbour];

                if (cell != ownCell && cell != enemyCell)
                {
                    hasEmptyNeighbour = true;
                    break;
                }
            }

            if (!hasEmptyNeighbour)
                ++blocked;
        }

        return blocked;
    }

    // Пробуем блокировать создание мельницы противником
    int countImmediateMillTargets(const GameState& state, Player player) const
    {
        const Cell ownCell = cellForPlayer(player);
        const Cell enemyCell = cellForPlayer(opponent(player));

        std::array<bool, GameBoard::Size> targets{};

        for (const auto& mill : GameBoard::getMills())
        {
            int ownPieces = 0;
            int enemyPieces = 0;
            int emptyCells = 0;
            int emptyPosition = -1;

            for (const int position : mill)
            {
                const Cell cell = state.board[position];

                if (cell == ownCell)
                    ++ownPieces;
                else if (cell == enemyCell)
                    ++enemyPieces;
                else
                {
                    ++emptyCells;
                    emptyPosition = position;
                }
            }

            if (ownPieces != 2 || enemyPieces != 0 || emptyCells != 1)
                continue;

            // Учитываем только точки, куда игрок действительно может поставить или переместить свою фишку
            if (canReachTarget(state, player, emptyPosition))
                targets[emptyPosition] = true;
        }

        int count = 0;

        for (const bool target : targets)
        {
            if (target)
                ++count;
        }

        return count;
    }

    // Проверяем, может ли игрок добраться до конкретной точки с учётом правил перемещения
    bool canReachTarget(const GameState& state, Player player, int target) const
    {
        if (target < 0 || target >= GameBoard::Size)
            return false;

        if (state.board[target] != Cell::Empty)
            return false;

        if (state.piecesInHand[playerIndex(player)] > 0)
            return true;

        const Cell ownCell = cellForPlayer(player);

        int pieces = 0;

        for (const Cell cell : state.board)
        {
            if (cell == ownCell)
                ++pieces;
        }

        if (pieces < 3)
            return false;

        if (pieces == 3)
            return true;

        // При четырёх и более фишках нужна соседняя фишка, которая может переместиться в целевую точку.
        for (int position = 0; position < GameBoard::Size; ++position)
        {
            if (state.board[position] != ownCell)
                continue;

            for (const int neighbour : GameBoard::neighbours(position))
            {
                if (neighbour == target)
                    return true;
            }
        }

        return false;

    }

    // Потенциал создания мельниц следующим ходом
    int countMillCreatingTargets(NineMensMorris& game) const
    {
        const Player player = game.currentPlayer();

        std::array<bool, GameBoard::Size> targets{};

        const auto moves = game.legalMoves();

        for (const auto& move : moves)
        {
            if (move.to < 0 || move.to >= GameBoard::Size)
                continue;

            if (game.formsMill(move, player))
                targets[move.to] = true;
        }

        int count = 0;

        for (const bool target : targets)
        {
            if (target)
                ++count;
        }

        return count;
    }

    static Cell cellForPlayer(Player player)
    {
        return player == Player::Player0 ? Cell::Player0 : Cell::Player1;
    }

    static int playerIndex(Player player)
    {
        return static_cast<int>(player);
    }

private:
    static constexpr int WinScore = 100'000;
    int _searchDepth;
};
#include "NineMensMorris.h"
#include "Minimax.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace {

void printBoard(const NineMensMorris& game)
{
    struct Coord {
        int r;
        int c;
    };

    static const std::array<Coord, 24> coords = {{
        {0, 0}, {0, 3}, {0, 6}, {3, 6}, {6, 6}, {6, 3}, {6, 0}, {3, 0},
        {1, 1}, {1, 3}, {1, 5}, {3, 5}, {5, 5}, {5, 3}, {5, 1}, {3, 1},
        {2, 2}, {2, 3}, {2, 4}, {3, 4}, {4, 4}, {4, 3}, {4, 2}, {3, 2}
    }};

    constexpr int Rows = 14;
    constexpr int Cols = 43;
    constexpr int RowStep = 2;
    constexpr int ColStep = 6;

    std::vector<std::string> canvas(Rows, std::string(Cols, ' '));

    for (int pos = 0; pos < GameBoard::Size; ++pos) {
        for (int neighbour : GameBoard::neighbours(pos)) {
            if (neighbour == -1 || neighbour < pos)
                continue;

            const Coord a = coords[pos];
            const Coord b = coords[neighbour];

            if (a.r == b.r) {
                const int row = a.r * RowStep + 1;
                const int left = std::min(a.c, b.c);
                const int right = std::max(a.c, b.c);

                for (int col = left * ColStep + 5;
                     col <= right * ColStep; ++col) {
                    if (col >= 0 && col < Cols)
                        canvas[row][col] = '-';
                }
            } else if (a.c == b.c) {
                const int col = a.c * ColStep + 3;
                const int top = std::min(a.r, b.r);
                const int bottom = std::max(a.r, b.r);

                for (int row = top * RowStep + 2;
                     row <= bottom * RowStep; ++row) {
                    if (row >= 0 && row < Rows)
                        canvas[row][col] = '|';
                }
            }
        }
    }

    for (int pos = 0; pos < GameBoard::Size; ++pos) {
        const Coord coord = coords[pos];
        const int row = coord.r * RowStep + 1;
        const int col = coord.c * ColStep + 1;

        std::string label;

        switch (game.cell(pos)) {
            case Cell::Player0:
                label = "[P0]";
                break;
            case Cell::Player1:
                label = "[P1]";
                break;
            default:
                label = pos < 10
                    ? "[0" + std::to_string(pos) + "]"
                    : "[" + std::to_string(pos) + "]";
                break;
        }

        for (int i = 0; i < static_cast<int>(label.size()); ++i)
            canvas[row][col + i] = label[i];
    }

    std::cout << '\n';

    for (const auto& line : canvas)
        std::cout << "  " << line << '\n';

    std::cout << '\n';
}

std::string trim(const std::string& s)
{
    const auto first = std::find_if_not(
        s.begin(), s.end(),
        [](unsigned char ch) { return std::isspace(ch); });

    if (first == s.end())
        return {};

    const auto last = std::find_if_not(
        s.rbegin(), s.rend(),
        [](unsigned char ch) { return std::isspace(ch); });

    return std::string(first, last.base());
}

std::optional<Move> parseMove(
    const std::string& line,
    const NineMensMorris& game)
{
    std::istringstream iss(line);
    std::vector<int> nums;
    int value;

    while (iss >> value)
        nums.push_back(value);

    if (!iss.eof() || nums.empty() || nums.size() > 3)
        return std::nullopt;

    Move move{};

    if (nums.size() == 1) {
        // Выставление фишки.
        move.from = -1;
        move.to = nums[0];
        move.capture = -1;
    } else if (nums.size() == 2) {
        // Перемещение фишки.
        move.from = nums[0];
        move.to = nums[1];
        move.capture = -1;
    } else {
        // Перемещение или выставление со взятием.
        move.from = nums[0];
        move.to = nums[1];
        move.capture = nums[2];
    }

    if (!game.isLegalMove(move))
        return std::nullopt;

    return move;
}

// Вывод хода бота: строго одна строка в stderr.
void printMove(const Move& move)
{
    if (move.from == -1 && move.capture == -1) {
        std::cerr << move.to << '\n';
    } else if (move.from == -1) {
        std::cerr << -1 << ' '
                  << move.to << ' '
                  << move.capture << '\n';
    } else if (move.capture == -1) {
        std::cerr << move.from << ' '
                  << move.to << '\n';
    } else {
        std::cerr << move.from << ' '
                  << move.to << ' '
                  << move.capture << '\n';
    }

    std::cerr.flush();
}

} // namespace

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <0|1>\n";
        return 1;
    }

    int color;

    try {
        std::size_t parsed = 0;
        const std::string argument = argv[argc - 1];

        color = std::stoi(argument, &parsed);

        if (parsed != argument.size()) {
            std::cout << "Color must be 0 or 1\n";
            return 1;
        }
    } catch (...) {
        std::cout << "Color must be 0 or 1\n";
        return 1;
    }

    if (color != 0 && color != 1) {
        std::cout << "Color must be 0 or 1\n";
        return 1;
    }

    const Player myPlayer =
        color == 0 ? Player::Player0 : Player::Player1;

    NineMensMorris game;
    GameBot ai(4);

    if (game.currentPlayer() != myPlayer)
        printBoard(game);

    while (true) {
        // Проверяем завершение игры перед запросом следующего хода.
        if (game.isGameOver()) {
            const auto winner = game.winner();

            if (!winner.has_value()) {
                std::cout << "Draw!\n";
                return 4;
            }

            if (*winner == myPlayer) {
                std::cout << "Bot wins!\n";
                return 0;
            }

            std::cout << "Bot loses!\n";
            return 3;
        }

        if (game.currentPlayer() == myPlayer) {
            const auto move = ai.findBestMove(game);

            if (!move) {
                // При отсутствии хода определяем результат по состоянию игры.
                if (game.isGameOver()) {
                    const auto winner = game.winner();

                    if (winner.has_value())
                        return *winner == myPlayer ? 0 : 3;

                    return 4;
                }

                std::cout << "AI could not find a move\n";
                return 3;
            }

            if (!game.makeMove(*move)) {
                std::cout << "AI produced illegal move\n";
                return 2;
            }

            printMove(*move);
            printBoard(game);
        } else {
            std::cout << "Enter coordinates: " << std::flush;

            std::string line;

            if (!std::getline(std::cin, line))
                return 3;

            line = trim(line);

            if (line.empty())
                continue;

            if (line == "undo" || line == "u") {
                if (game.undo()) {
                    if (game.currentPlayer() == myPlayer)
                        game.undo();

                    printBoard(game);
                }

                continue;
            }

            if (line == "undo2") {
                if (game.undo()) {
                    game.undo();
                    printBoard(game);
                }

                continue;
            }

            const auto move = parseMove(line, game);

            if (!move) {
                std::cout << "Invalid move. Try again.\n";
                continue;
            }

            if (!game.makeMove(*move)) {
                std::cout << "Failed to apply move. Try again.\n";
                continue;
            }

            if (move->from == -1) {
                std::cout << "Opponent placed a piece at cell "
                          << move->to << '\n';
            } else {
                std::cout << "Opponent moved from cell "
                          << move->from << " to cell "
                          << move->to << '\n';
            }

            if (move->capture != -1) {
                std::cout << "Opponent captured piece at cell "
                          << move->capture << '\n';
            }
        }
    }
}


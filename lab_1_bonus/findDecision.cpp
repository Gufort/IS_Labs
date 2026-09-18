#include "findDecision.h"

std::vector<Domino> dominoes;
std::mt19937 generator(static_cast<unsigned int>(
    std::chrono::steady_clock::now().time_since_epoch().count()));

// Идея состоит в том, чтобы считать штраф относительно 13 по строкам, столбцам и диагоналям
int calculateScore(const std::array<int,
    DOMINO_COUNT>& order, const std::array<bool,
    USED_DOMINOES>& flipped, std::array<int, BOARD_SIZE * BOARD_SIZE>* result)
{
    std::array<int, BOARD_SIZE * BOARD_SIZE> grid{};

    for (int position = 0; position < USED_DOMINOES; position++)
    {
        Domino domino = dominoes[order[position]];
        int top; int bottom;

        if (flipped[position]){
            top = domino.b;
            bottom = domino.a;
        } else {
            top = domino.a;
            bottom = domino.b;
        }

        int row = position / BOARD_SIZE;
        int column = position % BOARD_SIZE;

        int firstRow = row * 2;
        int secondRow = firstRow + 1;

        grid[firstRow * BOARD_SIZE + column] = top;
        grid[secondRow * BOARD_SIZE + column] = bottom;
    }

    // Далее займемся подсчетом штрафа (работаем с квадратами, чтобы нормально обрабатывать -2 и +2 разницы в разных строках)
    // Строки
    int score = 0;
    for (int row = 0; row < BOARD_SIZE; row++)
    {
        int sum = 0;
        for (int col = 0; col < BOARD_SIZE; col++)
        {
            sum += grid[row * BOARD_SIZE + col];
        }
        int diff = sum - TARGET;
        score += diff * diff;
    }

    // Столбцы
    for (int col = 0; col < BOARD_SIZE; col++)
    {
        int sum = 0;
        for (int row = 0; row < BOARD_SIZE; row++)
        {
            sum += grid[row * BOARD_SIZE + col];
        }
        int diff = sum - TARGET;
        score += diff * diff;
    }

    // Диагонали
    int mainDiagonal = 0;
    int secondDiagonal = 0;
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        mainDiagonal += grid[i * BOARD_SIZE + i];
        secondDiagonal += grid[i * BOARD_SIZE + (BOARD_SIZE - 1 - i)];
    }
    int diff = mainDiagonal - TARGET;
    score += diff * diff;
    diff = secondDiagonal - TARGET;
    score += diff * diff;

    if (result != nullptr)
        *result = grid;

    return score;
}

// Основной алгоритм
bool simulatedAnnealing(std::array<int, DOMINO_COUNT>& bestOrder,
                        std::array<bool, USED_DOMINOES>& bestFlipped)
{
    int steps = 5000000;
    int startTemp = 20.0;

    // Перемешиваем содержимое массива и назначаем случайную ориентацию в пространстве(0 - перевернута, 1 - нет)
    std::iota(bestOrder.begin(), bestOrder.end(), 0);
    std::shuffle(bestOrder.begin(), bestOrder.end(), generator);
    for (int i = 0; i < USED_DOMINOES; i++)
        bestFlipped[i] = generator() % 2;

    int currentScore = calculateScore(bestOrder, bestFlipped);

    for (int step = 0; step < steps; step++)
    {
        if (currentScore == 0) return true;

        double temperature = startTemp * (1.0 - double(step) / steps);

        // Копия текущего состояния
        std::array<int, DOMINO_COUNT> newOrder = bestOrder;
        std::array<bool, USED_DOMINOES> newFlipped = bestFlipped;

        int pos = generator() % USED_DOMINOES;
        if (generator() % 2)
            newFlipped[pos] = !newFlipped[pos];
        else
            std::swap(newOrder[pos], newOrder[generator() % DOMINO_COUNT]);

        int newScore = calculateScore(newOrder, newFlipped);

        double r = double(generator()) / generator.max();
        if (newScore <= currentScore || r < exp(double(currentScore - newScore) / temperature))
        {
            bestOrder = newOrder;
            bestFlipped = newFlipped;
            currentScore = newScore;
        }
    }
    return false;
}


void printResult(const std::array<int, DOMINO_COUNT>& order, const std::array<bool, USED_DOMINOES>& flipped)
{
    std::array<int, BOARD_SIZE * BOARD_SIZE> board;
    int score =calculateScore(order,flipped,&board);
    std::cout << "\nПоле:\n";
    for (int row = 0; row < BOARD_SIZE; row++)
    {
        int sum = 0;
        for (int col = 0; col < BOARD_SIZE; col++)
        {
            int value = board[row * BOARD_SIZE + col];
            std::cout << value << " ";
            sum += value;
        }
        std::cout << " | сумма = " << sum << '\n';
        if (row == 1 || row == 3)
        {
            std::cout << "-------------------------\n";
        }
    }
    std::cout << "\nСуммы вертикальных рядов:\n";
    for (int col = 0; col < BOARD_SIZE; col++)
    {
        int sum = 0;
        for (int row = 0; row < BOARD_SIZE; row++)
        {
            sum += board[row * BOARD_SIZE + col];
        }
        std::cout << sum << " ";
    }
    int mainDiagonal = 0;
    int secondDiagonal = 0;
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        mainDiagonal += board[i * BOARD_SIZE + i];
        secondDiagonal +=board[i * BOARD_SIZE + (BOARD_SIZE - 1 - i)];
    }
    std::cout << "\n\nГлавная диагональ: " << mainDiagonal;
    std::cout << "\nПобочная диагональ: " << secondDiagonal;
    std::cout << "\nШтраф: " << score<< '\n';
    std::cout << "\nРасположение костяшек:\n";
    for (int block = 0; block < 3; block++)
    {
        for (int col = 0; col < BOARD_SIZE; col++)
        {
            int position =
                block * BOARD_SIZE + col;
            Domino d = dominoes[order[position]];
            if (flipped[position])
            {
                std::swap(d.a, d.b);
            }

            std::cout << "["<< d.a << "|" << d.b << "] ";
        }
        std::cout << '\n';
    }
}

#include "BabylonTower.h"

bool isValid(const State& state) {
    int count[5] = {};
    int dots = 0;
    for (int i = 1; i <= 25; ++i) {
        char v = state[i];
        if (v == '.') { ++dots; continue; }
        int col = colorIdx(v);
        if (col < 0) return false;
        ++count[col];
    }
    if (dots > 1) return false;
    if (dots == 0) {
        for (int i = 0; i < 5; ++i) if (count[i] != 5) return false;
    } else {
        int fours = 0;
        for (int i = 0; i < 5; ++i) {
            if (count[i] == 4) ++fours;
            else if (count[i] != 5) return false;
        }
        if (fours != 1) return false;
    }
    return true;
}

/*

 */
static constexpr int PDB_TOP  = 7776;   // 6^5
static constexpr int PDB_SIZE = 15552;  // 2 * 6^5

static std::array<std::array<int16_t, PDB_SIZE>, 5> g_pdb;

// Работаем системе счисления по основанию 6
static inline int encodeCounts(
    const int cnt[5], // сколько шариков указанного цвета лежит в колонке, [0, 5]
    int top // флаг - находится ли шарик данного цвета в TOP
    )
{
    int code = top ? PDB_TOP : 0, mul = 1;
    for (int i = 0; i < 5; ++i)
    {
        code += cnt[i] * mul;
        mul *= 6;
    }
    return code;
}

static inline void decodeCounts(int code, int cnt[5], int& top) {
    top = (code >= PDB_TOP);
    if (top) code -= PDB_TOP;
    for (int i = 0; i < 5; ++i)
    {
        cnt[i] = code % 6;
        code /= 6;
    }
}

static void buildPDB()
{
    static bool built = false;
    if (built) return;
    built = true;

    // Построение таблицы для каждого из цветов
    for (int target = 0; target < 5; ++target)
    {
        auto& dist = g_pdb[target];
        dist.fill(-1); // берем табличку для текущего цвета, ни одно значение мы пока не посетили

        // Свой BFS-очередь на массиве — быстрее std::queue
        static int queueBuf[PDB_SIZE];
        int head = 0, tail = 0;

        int cnt[5] = {}; cnt[target] = 5; // Логично, что целевым будет столбец с 5 шарами одного цвета
        int start = encodeCounts(cnt, 0);
        dist[start] = 0; // ибо за 0 шагов в это состояние придем, ага ого
        queueBuf[tail++] = start;

        while (head < tail)
        {
            int code = queueBuf[head++];
            int d = dist[code], top, curr[5];
            decodeCounts(code, curr, top);

            // Имеем два направления сдвига
            for (int dir = -1; dir <= 1; dir += 2)
            {
                for (int mask = 1; mask < 32; ++mask) // 2^5 = 32, маска (какие колонки принимают участие в сдвиге)
                {
                    int new_cnt[5]; bool flag = true;
                    for (int i = 0; i < 5; ++i)
                        new_cnt[i] = curr[i];

                    // Гуляем по колонкам и ищем шарики нашего цвета
                    for (int column = 0; column < 5; ++column)
                    {
                        if (mask & (1 << column))
                        {
                            if (new_cnt[column] == 0)
                            {
                                flag = false;
                                break;
                            }
                            --new_cnt[column];
                        }
                    }
                    if (!flag) continue;

                    // Кладем шарик в соседний столбик, учитывая с помощью %, что столбиков то у нас 5 всего
                    for (int column = 0; column < 5; ++column)
                        if (mask & (1 << column))
                            ++new_cnt[(column + dir + 5) % 5];

                    for (int i = 0; i < 5; ++i)
                    {
                        if (new_cnt[i] > 5)
                        {
                            flag = false;
                            break;
                        }
                    }
                    if (!flag) continue;

                    // Кодируем новое состояние и добавляем в таблицу его, если его не было
                    int new_code = encodeCounts(new_cnt, top);
                    if (dist[new_code] == -1)
                    {
                        dist[new_code] = (int16_t)(d + 1);
                        queueBuf[tail++] = new_code;
                    }
                }
            }

            // Если манипулируем с пустой ячейкой
            if (top)
            {
                int new_cnt[5];
                for (int i = 0; i < 5; ++i)
                    new_cnt[i] = curr[i];

                // работа с пустой ячейкой проводится через нулевую колонку
                ++new_cnt[0];
                if (new_cnt[0] <= 5)
                {
                    int new_code = encodeCounts(new_cnt, 0);
                    if (dist[new_code] == -1)
                    {
                        dist[new_code] = (int16_t)(d + 1);
                        queueBuf[tail++] = new_code;
                    }
                }
            }
            else if (curr[0] > 0)
            {
                int new_cnt[5];
                for (int i = 0; i < 5; ++i)
                    new_cnt[i] = curr[i];
                --new_cnt[0];
                int new_code = encodeCounts(new_cnt, 1);
                if (dist[new_code] == -1)
                {
                    dist[new_code] = (int16_t)(d + 1);
                    queueBuf[tail++] = new_code;
                }
            }
        }
    }
}

int heuristic(const State& state)
{
    buildPDB();

    int cnt[5][5] = {}; // сколько дисков цвета i в столбце j
    char topChar = state[TOP];
    int topColor = (topChar == '.') ? -1 : colorIdx(topChar);

    // После циклов cnt[color][k] содержит распределение дисков каждого цвета по колонкам
    // Один проход по всем 25 позициям: column = (i-1) % 5
    for (int i = 1; i <= 25; ++i) {
        char v = state[i];
        if (v == '.') continue;
        int color = colorIdx(v);
        if (color < 0) continue;
        int column = (i - 1) % POSITIONS;
        ++cnt[color][column];
    }

    int sum = 0, max = 0;
    for (int color = 0; color < 5; ++color)
    {
        int code = encodeCounts(cnt[color], topColor == color ? 1 : 0);
        int d = g_pdb[color][code];
        if (d < 0) d = 0;
        sum += d;
        max = std::max(max, d);
    }

    return std::max(max, (sum + 4) / 5);
}

static bool isReverse(const Move& a, const Move& b) {
    if (a.type != b.type || a.index != b.index) return false;
    if (a.type == MoveType::SwapTop) return true;
    return a.direction == -b.direction;
}

// Глобальная таблица транспозиций для IDA*: для данного состояния храним
// максимальную оставшуюся глубину, для которой поддерево уже полностью
// исследовано и не дало решения.
static std::unordered_map<uint64_t, int> g_maxRem;

// Вспомогательная функция для рассматриваемых алгоритмов поиска
static bool dfsHelper(BabylonTower& tower, int depth, int limit,
    std::vector<Move>& path,
    const std::optional<Move>& prev, int h)
{
    if (tower.isSolved())          return true;
    if (depth + h > limit)     return false;

    uint64_t key = pack(tower.state());
    int rem = limit - depth;

    auto it = g_maxRem.find(key);
    if (it != g_maxRem.end() && it->second >= rem) return false;
    g_maxRem[key] = rem;

    // Ищем самые перспективные пути по h
    std::pair<int, Move> cand[32];
    int n = 0;

    Move buf[32];
    int m = tower.moves(buf);
    for (int i = 0; i < m; ++i)
    {
        if (prev && isReverse(*prev, buf[i])) continue;

        tower.applyMove(buf[i]);
        int new_h = heuristic(tower.state());
        tower.undoMove(buf[i]);

        if (depth + 1 + new_h > limit) continue;
        cand[n++] = { new_h, buf[i] };
    }

    // Сортируем по эвристике (вставками — для ≤20 элементов быстрее std::sort)
    for (int i = 1; i < n; ++i) {
        auto key2 = cand[i];
        int j = i - 1;
        while (j >= 0 && cand[j].first > key2.first) {
            cand[j + 1] = cand[j];
            --j;
        }
        cand[j + 1] = key2;
    }

    for (int i = 0; i < n; ++i)
    {
        path.push_back(cand[i].second);
        tower.applyMove(cand[i].second);
        if (dfsHelper(tower, depth + 1, limit, path, cand[i].second, cand[i].first))
            return true;
        tower.undoMove(cand[i].second);
        path.pop_back();
    }
    return false;
}

// DFS
int dfs(BabylonTower& tower, std::vector<Move>& solution, int limit)
{
    solution.clear();

    if (tower.isSolved())
        return 0;

    g_maxRem.clear();
    g_maxRem.reserve(1 << 16);

    if (dfsHelper(tower, 0, limit, solution, std::nullopt, heuristic(tower.state())))
        return static_cast<int>(solution.size());

    return -1;
}

// IDS
int ids(BabylonTower& tower, std::vector<Move>& solution, int maxDepth)
{
    solution.clear();

    if (tower.isSolved())
        return 0;

    int h0 = heuristic(tower.state());

    // Таблица НЕ очищается между итерациями: если поддерево уже исследовано
    // при меньшей (или равной) оставшейся глубине, повторно идти туда не нужно.
    g_maxRem.clear();
    g_maxRem.reserve(1 << 20);

    for (int limit = h0; limit <= maxDepth; ++limit)
    {
        std::vector<Move> path;
        path.reserve(limit);
        if (dfsHelper(tower, 0, limit, path, std::nullopt, h0)) {
            solution = std::move(path);
            return (int)solution.size();
        }
    }
    return -1;
}

// A*
int astar(BabylonTower& start, std::vector<Move>& solution, int maxDepth)
{
    solution.clear();
    buildPDB();

    if (start.isSolved())
        return 0;

    struct Node { State state;
        int g, parent; // parent — индекс родителя в векторе nodes (-1 у корня), g - стоимость пути от старта до текущей
        Move move; Move prev; bool hasPrev; };
    struct Item { int f, g, idx; }; // Для оценки
    struct Cmp  { bool operator()(const Item& a, const Item& b) const {
        return a.f != b.f ? a.f > b.f : a.g < b.g;
    }};

    std::vector<Node> nodes;
    nodes.reserve(1 << 16);

    std::unordered_map<uint64_t, int> bestG;
    bestG.reserve(1 << 16);

    std::priority_queue<Item, std::vector<Item>, Cmp> pq;

    nodes.push_back({ start.state(), 0, -1, Move{}, Move{}, false });
    bestG.emplace(pack(start.state()), 0);
    pq.push({ heuristic(start.state()), 0, 0 });

    BabylonTower tower = start;

    while (!pq.empty())
    {
        Item top = pq.top(); pq.pop();
        const int g = top.g, idx = top.idx;

        uint64_t key = pack(nodes[idx].state);
        auto bit = bestG.find(key);
        if (bit != bestG.end() && bit->second < g) continue; // Проверка на устаревшую запись

        tower = BabylonTower(nodes[idx].state);
        if (tower.isSolved()) {
            std::vector<Move> rev;
            for (int i = idx; nodes[i].parent != -1; i = nodes[i].parent)
                rev.push_back(nodes[i].move);
            std::reverse(rev.begin(), rev.end());
            solution = std::move(rev);
            return (int)solution.size();
        }
        if (g >= maxDepth) continue;

        Move buf[32];
        int m = tower.moves(buf);
        for (int k = 0; k < m; ++k)
        {
            if (nodes[idx].hasPrev && isReverse(nodes[idx].prev, buf[k])) continue;

            tower.applyMove(buf[k]);
            State ns = tower.state();
            tower.undoMove(buf[k]);

            uint64_t nkey = pack(ns);
            int ng = g + 1;
            auto nit = bestG.find(nkey);
            if (nit != bestG.end() && nit->second <= ng) continue;
            bestG[nkey] = ng;

            int nf = ng + heuristic(ns);
            if (nf > maxDepth) continue;

            nodes.push_back({ ns, ng, idx, buf[k], buf[k], true });
            pq.push({ nf, ng, (int)nodes.size() - 1 });
        }
    }
    return -1;
}
#include "BabylonTower.h"

bool isValid(const State& state) {
    int count[5] = {};
    int dots = 0;
    for (int i = 1; i <= 25; ++i) {
        char v = state[i];
        if (v == '.') { ++dots; continue; }
        int col = -1;
        for (int j = 0; j < 5; ++j) if (v == colors[j]) { col = j; break; }
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

int colorColumn(char color) {
    for (int c = 0; c < POSITIONS; ++c)
        if (colors[c] == color) return c;
    return -1;
}

/*

 */
static constexpr int PDB_TOP = 7776;
static constexpr int PDB_SIZE = 15552;

static std::array<std::array<int16_t, PDB_SIZE>, 5> g_pdb;
static bool isPdbBuilt = false;

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
    if (isPdbBuilt) return;
    isPdbBuilt = true;

    // Построение таблицы для каждого из цветов
    for (auto target = 0; target < 5; ++target)
    {
        auto& dist = g_pdb[target];
        dist.fill(-1); // берем табличку для текущего цвета, ни одно значение мы пока не посетили
        std::queue<int> q;

        int cnt[5] = {}; cnt[target] = 5; // Логично, что целевым будет столбец с 5 шарами одного цвета
        int start = encodeCounts(cnt, 0);
        dist[start] = 0; // ибо за 0 шагов в это состояние придем, ага ого
        q.push(start);

        while (!q.empty())
        {
            int code = q.front(); q.pop();
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
                        q.push(new_code);
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
                        q.push(new_code);
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
                    q.push(new_code);
                }
            }
        }
    }
}

int heuristic(const State& state)
{
    buildPDB();

    int cnt[5][5] = {}; // сколько дисков цвета i в столбце j
    int topColor = (state[TOP] == '.') ? -1 : colorColumn(state[TOP]);

    // После циклов cnt[color][k] содержит распределение дисков каждого цвета по колонкам
    for (int row = 0; row < DISCS; ++row)
    {
        for (int column = 0; column < POSITIONS; ++column)
        {
            char value = state[idx(row, column)];
            if (value == '.') continue;
            int color = colorColumn(value);
            if (color >= 0) ++cnt[color][column];
        }
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

// Вспомогательная функция для рассматриваемых алгоритмов поиска
static bool dfsHelper(BabylonTower& tower, int depth, int limit,
    std::unordered_map<State, int, StateHash>& best,
    std::vector<Move>& path,
    const std::optional<Move>& prev, int h)
{
    if (tower.isSolved())          return true;
    if (depth + h > limit)     return false;

    auto it = best.find(tower.state());
    if (it != best.end() && it->second <= depth) return false;
    best[tower.state()] = depth;

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

    // Сортируем по эвристике
    std::sort(cand, cand + n,
          [](const std::pair<int, Move>& a, const std::pair<int, Move>& b)
          {
              return a.first < b.first;
          });

    for (int i = 0; i < n; ++i)
    {
        path.push_back(cand[i].second);
        tower.applyMove(cand[i].second);
        if (dfsHelper(tower, depth + 1, limit, best, path, cand[i].second, cand[i].first))
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

    std::unordered_map<State, int, StateHash> best;

    if (dfsHelper(tower, 0, limit, best, solution, std::nullopt, heuristic(tower.state())))
        return static_cast<int>(solution.size());

    return -1;
}

// IDS
int ids(BabylonTower& tower, std::vector<Move>& solution, int maxDepth)
{
    solution.clear();

    if (tower.isSolved())
        return 0;

    for (int limit = heuristic(tower.state()); limit <= maxDepth; ++limit)
    {
        std::unordered_map<State, int, StateHash> best;
        std::vector<Move> path;
        if (dfsHelper(tower, 0, limit, best, path, std::nullopt, heuristic(tower.state()))) {
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
        Move move; };
    struct Item { int f, g, idx; }; // Для оценки
    struct Cmp  { bool operator()(const Item& a, const Item& b) const {
        return a.f != b.f ? a.f > b.f : a.g < b.g;
    }};

    std::vector<Node> nodes;
    std::unordered_map<State, int, StateHash> bestG;
    std::priority_queue<Item, std::vector<Item>, Cmp> pq;

    nodes.push_back({ start.state(), 0, -1, Move{} });
    bestG.emplace(start.state(), 0);
    pq.push({ heuristic(start.state()), 0, 0 });

    BabylonTower tower = start;

    while (!pq.empty())
    {
        Item top = pq.top(); pq.pop();
        const int g = top.g, idx = top.idx;

        auto bit = bestG.find(nodes[idx].state);
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
            tower.applyMove(buf[k]);
            State ns = tower.state();
            tower.undoMove(buf[k]);

            int ng = g + 1;
            auto nit = bestG.find(ns);
            if (nit != bestG.end() && nit->second <= ng) continue;
            bestG[ns] = ng;

            int nf = ng + heuristic(ns);
            if (nf > maxDepth + 1) continue;

            nodes.push_back({ ns, ng, idx, buf[k] });
            pq.push({ nf, ng, (int)nodes.size() - 1 });
        }
    }
}
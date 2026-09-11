#include "iostream"
#include "simple_test.h"
#include <unordered_map>
#include "queue"
#include "unordered_set"


int findMinSecOfOperation(int start, int end)
{
    std::queue<std::pair<int, int>> q;
    std::unordered_set<int> visited;

    visited.insert(start);
    q.push({start, 0});

    while (!q.empty())
    {
        auto [number, steps] = q.front();
        q.pop();

        if (number == end)
            return steps;

        int next = number + 3;

        if (next <= end && !visited.count(next))
        {
            visited.insert(next);
            q.push({next, steps + 1});
        }

        next = number * 2;

        if (next <= end && !visited.count(next))
        {
            visited.insert(next);
            q.push({next, steps + 1});
        }
    }

    return -1;
}


int findMinSecOfThreeOperation(int start, int end)
{
    std::queue<std::pair<int, int>> q;
    std::unordered_set<int> visited;

    visited.insert(start);
    q.push({start, 0});

    while (!q.empty())
    {
        auto [number, steps] = q.front();
        q.pop();

        if (number == end)
            return steps;

        int next = number + 3;

        if (next <= end && !visited.count(next))
        {
            visited.insert(next);
            q.push({next, steps + 1});
        }

        next = number * 2;

        if (next <= end && !visited.count(next))
        {
            visited.insert(next);
            q.push({next, steps + 1});
        }

        next = number - 2;

        if (next >= start && !visited.count(next))
        {
            visited.insert(next);
            q.push({next, steps + 1});
        }
    }

    return -1;
}


int findMinSecOfOperationReverse(int start, int end)
{
    std::queue<std::pair<int, int>> q;
    std::unordered_set<int> visited;

    visited.insert(end);
    q.push({end, 0});

    while (!q.empty())
    {
        auto [number, steps] = q.front();
        q.pop();

        if (number == start)
            return steps;

        int next = number - 3;

        if (next >= start && !visited.count(next))
        {
            visited.insert(next);
            q.push({next, steps + 1});
        }

        if (number % 2 == 0)
        {
            next = number / 2;

            if (next >= start && !visited.count(next))
            {
                visited.insert(next);
                q.push({next, steps + 1});
            }
        }

        next = number + 2;

        if (next <= start && !visited.count(next))
        {
            visited.insert(next);
            q.push({next, steps + 1});
        }
    }

    return -1;
}

int findMinSecOfOperationBidirectional(int start, int end)
{
    if (start == end)
        return 0;

    std::queue<int> qStart;
    std::queue<int> qEnd;

    std::unordered_map<int, int> distStart;
    std::unordered_map<int, int> distEnd;

    qStart.push(start);
    qEnd.push(end);

    distStart[start] = 0;
    distEnd[end] = 0;

    while (!qStart.empty() && !qEnd.empty())
    {
        int size = qStart.size();

        while (size--)
        {
            int number = qStart.front();
            qStart.pop();

            int next = number + 3;

            if (next <= end && !distStart.count(next))
            {
                distStart[next] = distStart[number] + 1;

                if (distEnd.count(next))
                    return distStart[next] + distEnd[next];

                qStart.push(next);
            }

            next = number * 2;

            if (next <= end && !distStart.count(next))
            {
                distStart[next] = distStart[number] + 1;

                if (distEnd.count(next))
                    return distStart[next] + distEnd[next];

                qStart.push(next);
            }
        }

        size = qEnd.size();

        while (size--)
        {
            int number = qEnd.front();
            qEnd.pop();

            int next = number - 3;

            if (next >= start && !distEnd.count(next))
            {
                distEnd[next] = distEnd[number] + 1;

                if (distStart.count(next))
                    return distStart[next] + distEnd[next];

                qEnd.push(next);
            }

            if (number % 2 == 0)
            {
                next = number / 2;

                if (next >= start && !distEnd.count(next))
                {
                    distEnd[next] = distEnd[number] + 1;

                    if (distStart.count(next))
                        return distStart[next] + distEnd[next];

                    qEnd.push(next);
                }
            }
        }
    }

    return -1;
}
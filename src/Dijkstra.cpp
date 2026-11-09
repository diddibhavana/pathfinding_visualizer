#include "pfv/Dijkstra.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <functional>
#include <limits>
#include <queue>
#include <vector>

namespace pfv {

namespace {

// In uniform grid graphs, each orthogonal step has an edge weight of 1
constexpr int kUnitCost = 1;
constexpr int kInfinity = std::numeric_limits<int>::max() / 4;

struct QueueEntry {
    int distance;
    int index;

    bool operator>(const QueueEntry& other) const noexcept {
        return distance > other.distance;
    }
};

} // namespace

SearchResult Dijkstra::solve(const Grid& grid) const {
    const auto startTime = std::chrono::steady_clock::now();

    SearchResult result;
    const int totalCells = grid.rows() * grid.cols();
    const int sourceIdx = grid.index(grid.start());
    const int targetIdx = grid.index(grid.end());

    std::vector<int> parent(static_cast<std::size_t>(totalCells), -1);
    std::vector<int> dist(static_cast<std::size_t>(totalCells), kInfinity);
    std::vector<std::uint8_t> closed(static_cast<std::size_t>(totalCells), 0);
    std::vector<std::uint8_t> discovered(static_cast<std::size_t>(totalCells), 0);

    // Min-priority queue: prioritizes smallest tentative distance
    std::priority_queue<QueueEntry, std::vector<QueueEntry>, std::greater<QueueEntry>> priorityQueue;

    dist[static_cast<std::size_t>(sourceIdx)] = 0;
    priorityQueue.push({0, sourceIdx});

    bool targetReached = false;

    while (!priorityQueue.empty()) {
        const auto [currentDist, currentIdx] = priorityQueue.top();
        priorityQueue.pop();

        // Skip stale entries or already processed nodes
        if (currentDist != dist[static_cast<std::size_t>(currentIdx)] ||
            closed[static_cast<std::size_t>(currentIdx)]) {
            continue;
        }

        closed[static_cast<std::size_t>(currentIdx)] = 1;
        const Coord currentCoord{currentIdx / grid.cols(), currentIdx % grid.cols()};

        if (currentIdx != sourceIdx && currentIdx != targetIdx) {
            result.events.push_back({currentCoord, NodeState::Closed});
            ++result.visitedCount;
        }

        if (currentIdx == targetIdx) {
            targetReached = true;
            break;
        }

        // Relax neighbors
        for (const Coord neighbor : grid.neighbors(currentCoord)) {
            const int neighborIdx = grid.index(neighbor);
            const int newDist = currentDist + kUnitCost;

            if (newDist < dist[static_cast<std::size_t>(neighborIdx)]) {
                dist[static_cast<std::size_t>(neighborIdx)] = newDist;
                parent[static_cast<std::size_t>(neighborIdx)] = currentIdx;
                priorityQueue.push({newDist, neighborIdx});

                if (!discovered[static_cast<std::size_t>(neighborIdx)]) {
                    discovered[static_cast<std::size_t>(neighborIdx)] = 1;
                    if (neighborIdx != targetIdx) {
                        result.events.push_back({neighbor, NodeState::Open});
                    }
                }
            }
        }
    }

    // Reconstruct the shortest path
    if (targetReached) {
        for (int i = targetIdx; i != sourceIdx; i = parent[static_cast<std::size_t>(i)]) {
            if (i < 0) {
                targetReached = false;
                break;
            }
            result.path.push_back({i / grid.cols(), i % grid.cols()});
        }

        std::reverse(result.path.begin(), result.path.end());

        for (const Coord cell : result.path) {
            if (cell != grid.start() && cell != grid.end()) {
                result.events.push_back({cell, NodeState::Path});
            }
        }
    }

    result.found = targetReached;
    result.pathLength = targetReached ? static_cast<int>(result.path.size()) + 1 : 0;

    const auto elapsed = std::chrono::steady_clock::now() - startTime;
    result.computeMs = std::chrono::duration<double, std::milli>(elapsed).count();

    return result;
}

const char* Dijkstra::name() const {
    return "Dijkstra";
}

} // namespace pfv

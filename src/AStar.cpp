#include "pfv/AStar.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <limits>
#include <queue>
#include <vector>

namespace pfv {

namespace {

constexpr int kInfinity = std::numeric_limits<int>::max() / 4;

/// Manhattan distance heuristic (L1 norm).
/// For a 4-connected grid with uniform edge weights, this heuristic is admissible and consistent.
inline int manhattanDistance(Coord a, Coord b) noexcept {
    return std::abs(a.row - b.row) + std::abs(a.col - b.col);
}

/// Priority queue element for A* frontier.
struct AStarEntry {
    int fScore{0};  // f(n) = g(n) + h(n)
    int hScore{0};  // h(n) heuristic estimate to goal
    int index{0};   // Flattened cell index

    // Min-heap ordering: lowest f-score first.
    // Tie-breaker: lowest h-score first (favor nodes closer to the target).
    bool operator>(const AStarEntry& other) const noexcept {
        if (fScore != other.fScore) {
            return fScore > other.fScore;
        }
        if (hScore != other.hScore) {
            return hScore > other.hScore;
        }
        return index > other.index;
    }
};

} // namespace

SearchResult AStar::solve(const Grid& grid) const {
    const auto startTime = std::chrono::steady_clock::now();

    SearchResult result;
    const int totalCells = grid.rows() * grid.cols();
    const int sourceIdx = grid.index(grid.start());
    const int targetIdx = grid.index(grid.end());

    std::vector<int> parent(static_cast<std::size_t>(totalCells), -1);
    std::vector<int> gScore(static_cast<std::size_t>(totalCells), kInfinity);
    std::vector<std::uint8_t> closed(static_cast<std::size_t>(totalCells), 0);
    std::vector<std::uint8_t> discovered(static_cast<std::size_t>(totalCells), 0);

    std::priority_queue<AStarEntry, std::vector<AStarEntry>, std::greater<AStarEntry>> openSet;

    gScore[static_cast<std::size_t>(sourceIdx)] = 0;
    const int initialH = manhattanDistance(grid.start(), grid.end());
    openSet.push({initialH, initialH, sourceIdx});

    bool targetReached = false;

    while (!openSet.empty()) {
        const AStarEntry current = openSet.top();
        openSet.pop();

        const int currentIdx = current.index;

        if (closed[static_cast<std::size_t>(currentIdx)]) {
            continue;
        }

        const Coord currentCoord{currentIdx / grid.cols(), currentIdx % grid.cols()};

        // Skip stale entries where g(n) was subsequently improved
        const int expectedF = gScore[static_cast<std::size_t>(currentIdx)] +
                              manhattanDistance(currentCoord, grid.end());
        if (current.fScore != expectedF) {
            continue;
        }

        closed[static_cast<std::size_t>(currentIdx)] = 1;

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
            const int tentativeG = gScore[static_cast<std::size_t>(currentIdx)] + 1;

            if (tentativeG < gScore[static_cast<std::size_t>(neighborIdx)]) {
                gScore[static_cast<std::size_t>(neighborIdx)] = tentativeG;
                parent[static_cast<std::size_t>(neighborIdx)] = currentIdx;

                const int h = manhattanDistance(neighbor, grid.end());
                openSet.push({tentativeG + h, h, neighborIdx});

                if (!discovered[static_cast<std::size_t>(neighborIdx)]) {
                    discovered[static_cast<std::size_t>(neighborIdx)] = 1;
                    if (neighborIdx != targetIdx) {
                        result.events.push_back({neighbor, NodeState::Open});
                    }
                }
            }
        }
    }

    // Reconstruct the optimal path
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

const char* AStar::name() const {
    return "A*";
}

} // namespace pfv

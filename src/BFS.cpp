#include "pfv/BFS.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <queue>
#include <vector>

namespace pfv {

SearchResult BFS::solve(const Grid& grid) const {
    const auto startTime = std::chrono::steady_clock::now();

    SearchResult result;
    const int totalCells = grid.rows() * grid.cols();
    const int sourceIdx = grid.index(grid.start());
    const int targetIdx = grid.index(grid.end());

    std::vector<int> parent(static_cast<std::size_t>(totalCells), -1);
    std::vector<std::uint8_t> visited(static_cast<std::size_t>(totalCells), 0);
    std::queue<int> frontier;

    frontier.push(sourceIdx);
    visited[static_cast<std::size_t>(sourceIdx)] = 1;

    bool targetReached = false;

    while (!frontier.empty()) {
        const int currentIdx = frontier.front();
        frontier.pop();

        const Coord currentCoord{currentIdx / grid.cols(), currentIdx % grid.cols()};

        // Record node expansion (closed/visited) event for animation
        if (currentIdx != sourceIdx && currentIdx != targetIdx) {
            result.events.push_back({currentCoord, NodeState::Closed});
            ++result.visitedCount;
        }

        if (currentIdx == targetIdx) {
            targetReached = true;
            break;
        }

        // Explore adjacent unvisited cardinal neighbors
        for (const Coord neighbor : grid.neighbors(currentCoord)) {
            const int neighborIdx = grid.index(neighbor);

            if (!visited[static_cast<std::size_t>(neighborIdx)]) {
                visited[static_cast<std::size_t>(neighborIdx)] = 1;
                parent[static_cast<std::size_t>(neighborIdx)] = currentIdx;
                frontier.push(neighborIdx);

                if (neighborIdx != targetIdx) {
                    result.events.push_back({neighbor, NodeState::Open});
                }
            }
        }
    }

    // Reconstruct the shortest path by backtracking from target to source
    if (targetReached) {
        for (int i = targetIdx; i != sourceIdx; i = parent[static_cast<std::size_t>(i)]) {
            if (i < 0) {
                targetReached = false;
                break;
            }
            result.path.push_back({i / grid.cols(), i % grid.cols()});
        }

        std::reverse(result.path.begin(), result.path.end());

        // Emit visualization events for the path (excluding start and end markers)
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

const char* BFS::name() const {
    return "BFS";
}

} // namespace pfv

#include "pfv/DFS.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <stack>
#include <vector>

namespace pfv {

SearchResult DFS::solve(const Grid& grid) const {
    const auto startTime = std::chrono::steady_clock::now();

    SearchResult result;
    const int totalCells = grid.rows() * grid.cols();
    const int sourceIdx = grid.index(grid.start());
    const int targetIdx = grid.index(grid.end());

    std::vector<int> parent(static_cast<std::size_t>(totalCells), -1);
    std::vector<std::uint8_t> closed(static_cast<std::size_t>(totalCells), 0);
    std::vector<std::uint8_t> discovered(static_cast<std::size_t>(totalCells), 0);

    std::stack<int> stack;
    stack.push(sourceIdx);
    discovered[static_cast<std::size_t>(sourceIdx)] = 1;

    bool targetReached = false;

    while (!stack.empty()) {
        const int currentIdx = stack.top();
        stack.pop();

        if (closed[static_cast<std::size_t>(currentIdx)]) {
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

        // Iterate neighbors in reverse so that pushing onto the LIFO stack
        // preserves the standard exploration order (top to bottom)
        const auto neighbors = grid.neighbors(currentCoord);
        for (auto it = neighbors.rbegin(); it != neighbors.rend(); ++it) {
            const int neighborIdx = grid.index(*it);

            if (!closed[static_cast<std::size_t>(neighborIdx)]) {
                if (parent[static_cast<std::size_t>(neighborIdx)] == -1 && neighborIdx != sourceIdx) {
                    parent[static_cast<std::size_t>(neighborIdx)] = currentIdx;
                }

                stack.push(neighborIdx);

                if (!discovered[static_cast<std::size_t>(neighborIdx)]) {
                    discovered[static_cast<std::size_t>(neighborIdx)] = 1;
                    if (neighborIdx != targetIdx) {
                        result.events.push_back({*it, NodeState::Open});
                    }
                }
            }
        }
    }

    // Reconstruct the discovered path (note: DFS does not guarantee shortest path)
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

const char* DFS::name() const {
    return "DFS";
}

} // namespace pfv

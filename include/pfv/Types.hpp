#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <vector>

namespace pfv {

/// 2D grid coordinates (0-indexed).
struct Coord {
    int row{0};
    int col{0};
};

inline bool operator==(Coord lhs, Coord rhs) noexcept {
    return lhs.row == rhs.row && lhs.col == rhs.col;
}

inline bool operator!=(Coord lhs, Coord rhs) noexcept {
    return !(lhs == rhs);
}

/// Visual and logical state of a grid cell.
enum class NodeState : std::uint8_t {
    Unvisited,
    Wall,
    Start,
    End,
    Open,    // Node in frontier / priority queue
    Closed,  // Node visited / processed
    Path     // Final reconstructed path
};

/// Supported pathfinding algorithms.
enum class AlgorithmType {
    Dijkstra,
    AStar,
    BFS,
    DFS
};

/// A recorded state change event used for step-by-step visual animation.
struct StepEvent {
    Coord cell;
    NodeState newState;
};

/// Result returned by pathfinding algorithm executions.
struct SearchResult {
    std::vector<StepEvent> events;  // Chronological stream of exploration & path events
    std::vector<Coord> path;        // Sequence of coordinates from start to end (exclusive)
    bool found{false};              // True if a path to target was discovered
    int visitedCount{0};            // Total nodes expanded / closed
    int pathLength{0};              // Total edge count of the found path
    double computeMs{0.0};          // Execution time in milliseconds
};

} // namespace pfv

// Custom hash specialization so Coord can be used in std::unordered_set / map
namespace std {
template <>
struct hash<pfv::Coord> {
    std::size_t operator()(pfv::Coord c) const noexcept {
        const auto r = static_cast<std::uint32_t>(c.row);
        const auto col = static_cast<std::uint32_t>(c.col);
        return static_cast<std::size_t>(r) * static_cast<std::size_t>(0x9e3779b1U) ^
               static_cast<std::size_t>(col);
    }
};
} // namespace std

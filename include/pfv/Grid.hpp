#pragma once

#include "pfv/Types.hpp"

#include <vector>

namespace pfv {

/// Represents a 2D uniform grid graph with obstacle and marker state.
class Grid {
public:
    explicit Grid(int rows = 40, int cols = 70);

    // Dimensions and bounds
    int rows() const noexcept { return rows_; }
    int cols() const noexcept { return cols_; }
    bool inBounds(Coord c) const noexcept;
    int index(Coord c) const;

    // Node state accessors
    NodeState get(Coord c) const;
    void set(Coord c, NodeState state);

    // Graph topology: returns valid 4-way cardinal neighbors (non-walls)
    std::vector<Coord> neighbors(Coord c) const;

    // Markers
    Coord start() const noexcept { return start_; }
    Coord end() const noexcept { return end_; }
    bool setStart(Coord c);
    bool setEnd(Coord c);

    // Wall manipulation
    bool setWall(Coord c, bool wall);

    // Reset operations
    void clearPath();
    void clearWalls();
    void reset();

private:
    int rows_{0};
    int cols_{0};
    Coord start_{0, 0};
    Coord end_{0, 0};
    std::vector<NodeState> cells_;
};

} // namespace pfv

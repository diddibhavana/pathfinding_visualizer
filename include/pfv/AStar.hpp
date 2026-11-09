#pragma once

#include "pfv/Pathfinder.hpp"

namespace pfv {

/// A* search algorithm using the Manhattan distance heuristic on 4-connected grid.
class AStar final : public IPathfinder {
public:
    SearchResult solve(const Grid& grid) const override;
    const char* name() const override;
};

} // namespace pfv

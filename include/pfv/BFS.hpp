#pragma once

#include "pfv/Pathfinder.hpp"

namespace pfv {

/// Breadth-First Search: guarantees shortest path on unweighted grids.
class BFS final : public IPathfinder {
public:
    SearchResult solve(const Grid& grid) const override;
    const char* name() const override;
};

} // namespace pfv

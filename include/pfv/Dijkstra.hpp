#pragma once

#include "pfv/Pathfinder.hpp"

namespace pfv {

/// Dijkstra's algorithm: priority-queue based shortest path search.
class Dijkstra final : public IPathfinder {
public:
    SearchResult solve(const Grid& grid) const override;
    const char* name() const override;
};

} // namespace pfv

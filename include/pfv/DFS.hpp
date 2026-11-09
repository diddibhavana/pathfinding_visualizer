#pragma once

#include "pfv/Pathfinder.hpp"

namespace pfv {

/// Depth-First Search: explores branches deeply; does not guarantee shortest path.
class DFS final : public IPathfinder {
public:
    SearchResult solve(const Grid& grid) const override;
    const char* name() const override;
};

} // namespace pfv

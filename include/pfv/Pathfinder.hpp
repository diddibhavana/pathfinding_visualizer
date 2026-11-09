#pragma once

#include "pfv/Grid.hpp"

#include <memory>

namespace pfv {

/// Abstract interface for pathfinding algorithm implementations.
class IPathfinder {
public:
    virtual ~IPathfinder() = default;

    /// Executes the search on the given grid and returns the exploration events & path.
    virtual SearchResult solve(const Grid& grid) const = 0;

    /// Human-readable display name of the algorithm.
    virtual const char* name() const = 0;
};

/// Factory function to instantiate pathfinding algorithms.
std::unique_ptr<IPathfinder> makePathfinder(AlgorithmType type);

} // namespace pfv

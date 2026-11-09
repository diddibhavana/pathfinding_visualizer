#include "pfv/Pathfinder.hpp"

#include "pfv/AStar.hpp"
#include "pfv/BFS.hpp"
#include "pfv/DFS.hpp"
#include "pfv/Dijkstra.hpp"

#include <stdexcept>

namespace pfv {

std::unique_ptr<IPathfinder> makePathfinder(AlgorithmType type) {
    switch (type) {
    case AlgorithmType::Dijkstra:
        return std::make_unique<Dijkstra>();
    case AlgorithmType::AStar:
        return std::make_unique<AStar>();
    case AlgorithmType::BFS:
        return std::make_unique<BFS>();
    case AlgorithmType::DFS:
        return std::make_unique<DFS>();
    }

    throw std::invalid_argument("Unknown pathfinding algorithm type");
}

} // namespace pfv

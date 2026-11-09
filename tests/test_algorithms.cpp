#include "pfv/Pathfinder.hpp"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <random>
#include <vector>

namespace {

using namespace pfv;

int manhattanDistance(Coord a, Coord b) {
    return std::abs(a.row - b.row) + std::abs(a.col - b.col);
}

void testShortestPathConsistencyAndOptimality() {
    std::cout << "[TEST] Running testShortestPathConsistencyAndOptimality...\n";

    const auto bfs = makePathfinder(AlgorithmType::BFS);
    const auto dijkstra = makePathfinder(AlgorithmType::Dijkstra);
    const auto astar = makePathfinder(AlgorithmType::AStar);
    const auto dfs = makePathfinder(AlgorithmType::DFS);

    std::mt19937 rng(91357U);
    std::uniform_real_distribution<double> distribution(0.0, 1.0);

    for (int trial = 0; trial < 20; ++trial) {
        Grid sample(20, 30);

        for (int r = 0; r < sample.rows(); ++r) {
            for (int c = 0; c < sample.cols(); ++c) {
                const Coord cell{r, c};
                if (cell != sample.start() && cell != sample.end() && distribution(rng) < 0.24) {
                    sample.setWall(cell, true);
                }
            }
        }

        const auto resBFS = bfs->solve(sample);
        const auto resDijkstra = dijkstra->solve(sample);
        const auto resAStar = astar->solve(sample);
        const auto resDFS = dfs->solve(sample);

        // BFS, Dijkstra, and A* must agree on whether a path exists
        assert(resBFS.found == resDijkstra.found);
        assert(resBFS.found == resAStar.found);

        if (resBFS.found) {
            // BFS, Dijkstra, and A* must all find optimal paths of equal length
            assert(resBFS.pathLength == resDijkstra.pathLength);
            assert(resBFS.pathLength == resAStar.pathLength);
            assert(resDFS.found);

            // Verify path continuity and obstacle avoidance
            Coord previous = sample.start();
            for (const Coord cell : resBFS.path) {
                const int dist = manhattanDistance(previous, cell);
                assert(dist == 1);
                assert(sample.get(cell) != NodeState::Wall);
                previous = cell;
            }
            assert(previous == sample.end());
        } else {
            assert(!resDFS.found);
        }
    }

    std::cout << "  -> Passed: 20 random grid trials verified.\n";
}

void testUnreachableTarget() {
    std::cout << "[TEST] Running testUnreachableTarget...\n";

    const auto bfs = makePathfinder(AlgorithmType::BFS);
    const auto dijkstra = makePathfinder(AlgorithmType::Dijkstra);
    const auto astar = makePathfinder(AlgorithmType::AStar);

    Grid blocked(5, 5);
    const Coord end = blocked.end();

    // Wall off all adjacent neighbors of the destination
    for (const Coord neighbor : blocked.neighbors(end)) {
        blocked.setWall(neighbor, true);
    }

    assert(!bfs->solve(blocked).found);
    assert(!dijkstra->solve(blocked).found);
    assert(!astar->solve(blocked).found);

    std::cout << "  -> Passed: Unreachable target correctly detected.\n";
}

void testAStarHeuristicEfficiency() {
    std::cout << "[TEST] Running testAStarHeuristicEfficiency...\n";

    const auto dijkstra = makePathfinder(AlgorithmType::Dijkstra);
    const auto astar = makePathfinder(AlgorithmType::AStar);

    Grid openGrid(20, 30);
    const auto resDijkstra = dijkstra->solve(openGrid);
    const auto resAStar = astar->solve(openGrid);

    // On an unweighted obstacle-free grid, A* with Manhattan heuristic
    // must expand fewer or equal nodes compared to uninformed Dijkstra
    assert(resAStar.visitedCount <= resDijkstra.visitedCount);

    std::cout << "  -> Passed: A* explored " << resAStar.visitedCount
              << " cells vs Dijkstra " << resDijkstra.visitedCount << " cells.\n";
}

void testGridMutationsAndBoundaryConstraints() {
    std::cout << "[TEST] Running testGridMutationsAndBoundaryConstraints...\n";

    Grid grid(8, 10);

    // Start and End cells cannot be turned into walls
    assert(!grid.setWall(grid.start(), true));
    assert(!grid.setWall(grid.end(), true));

    // Normal cell wall toggling
    const Coord wallPos{0, 0};
    assert(grid.setWall(wallPos, true));

    // Clear path must preserve walls and markers
    grid.set(wallPos, NodeState::Path);
    grid.clearPath();
    assert(grid.get(wallPos) == NodeState::Unvisited);
    assert(grid.get(grid.start()) == NodeState::Start);
    assert(grid.get(grid.end()) == NodeState::End);

    std::cout << "  -> Passed: Grid constraints and clear logic verified.\n";
}

} // namespace

int main() {
    std::cout << "=== Running Pathfinding Visualizer Algorithm Tests ===\n";

    testShortestPathConsistencyAndOptimality();
    testUnreachableTarget();
    testAStarHeuristicEfficiency();
    testGridMutationsAndBoundaryConstraints();

    std::cout << "=== All tests passed successfully! ===\n";
    return 0;
}

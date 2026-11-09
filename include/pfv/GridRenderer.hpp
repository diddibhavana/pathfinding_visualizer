#pragma once

#include "pfv/Grid.hpp"

#include <SFML/Graphics.hpp>

#include <optional>
#include <vector>

namespace pfv {

/// Color palette for rendering the grid and HUD elements.
namespace palette {
    inline constexpr sf::Color background{30, 33, 39};    // Window background
    inline constexpr sf::Color cell{43, 47, 56};          // Unvisited empty cell
    inline constexpr sf::Color gridLine{23, 26, 31};      // Grid border lines
    inline constexpr sf::Color wall{11, 13, 16};          // Impassable wall
    inline constexpr sf::Color start{46, 204, 113};       // Start marker (emerald green)
    inline constexpr sf::Color end{231, 76, 60};          // Target marker (crimson red)
    inline constexpr sf::Color open{241, 196, 15};        // Frontier / open set (sunflower yellow)
    inline constexpr sf::Color closed{45, 125, 185};      // Visited / closed set (ocean blue)
    inline constexpr sf::Color path{243, 156, 18};        // Final shortest path (warm amber)
} // namespace palette

/// Efficient batched vertex array renderer for the 2D grid.
class GridRenderer {
public:
    GridRenderer(int rows, int cols);

    /// Adapts cell sizing and centering whenever the window dimensions change.
    void resize(const sf::RenderWindow& window);

    /// Updates vertex colors for all cells in the grid.
    void rebuildAll(const Grid& grid);

    /// Marks a cell as needing its vertex color updated.
    void markDirty(Coord c);

    /// Flushes pending dirty cell color updates to the vertex array.
    void flush(const Grid& grid);

    /// Draws the batched cell quads and grid lines.
    void draw(sf::RenderWindow& window) const;

    /// Converts a pixel coordinate from the window to grid (row, col) coordinates.
    std::optional<Coord> screenToCell(sf::Vector2i point, const sf::RenderWindow& window) const;

    float top() const noexcept { return top_; }

private:
    sf::Color colorFor(NodeState state) const;
    void rebuildGeometry();

    int rows_{0};
    int cols_{0};
    float top_{56.0F};     // Height reserved for the top HUD bar
    float left_{0.0F};     // Left offset for horizontal centering
    float cellW_{1.0F};
    float cellH_{1.0F};

    sf::VertexArray cells_{sf::Quads};
    sf::VertexArray lines_{sf::Lines};
    std::vector<Coord> dirty_;
};

} // namespace pfv

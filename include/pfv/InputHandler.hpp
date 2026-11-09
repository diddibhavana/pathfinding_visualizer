#pragma once

#include "pfv/GridRenderer.hpp"

#include <SFML/Window.hpp>

namespace pfv {

/// Handles interactive mouse drag input: wall drawing, wall erasing, and start/end marker moving.
class InputHandler {
public:
    /// Processes mouse events. Returns true if the grid was modified.
    bool handle(const sf::Event& event,
                const sf::RenderWindow& window,
                Grid& grid,
                const GridRenderer& renderer,
                bool editingAllowed,
                bool& markerMoved);

    void reset() noexcept;

private:
    enum class DragMode {
        None,
        DrawWall,
        EraseWall,
        DragStart,
        DragEnd,
        RightErase
    };

    // Bresenham's line algorithm to paint continuous lines during rapid mouse movement
    void applyLine(Coord from, Coord to, Grid& grid, bool& markerMoved);

    DragMode mode_{DragMode::None};
    Coord previousCoord_{};
    bool hasPrevious_{false};
};

} // namespace pfv

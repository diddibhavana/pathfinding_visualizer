#include "pfv/InputHandler.hpp"

#include <cstdlib>

namespace pfv {

void InputHandler::reset() noexcept {
    mode_ = DragMode::None;
    hasPrevious_ = false;
}

void InputHandler::applyLine(Coord from, Coord to, Grid& grid, bool& markerMoved) {
    // Bresenham's line algorithm: ensures continuous painting without gaps
    // when dragging the mouse across multiple cells between frames
    int x0 = from.col;
    int y0 = from.row;
    const int x1 = to.col;
    const int y1 = to.row;

    const int dx = std::abs(x1 - x0);
    const int sx = x0 < x1 ? 1 : -1;
    const int dy = -std::abs(y1 - y0);
    const int sy = y0 < y1 ? 1 : -1;
    int error = dx + dy;

    while (true) {
        const Coord cell{y0, x0};

        switch (mode_) {
        case DragMode::DrawWall:
            grid.setWall(cell, true);
            break;
        case DragMode::EraseWall:
        case DragMode::RightErase:
            grid.setWall(cell, false);
            break;
        case DragMode::DragStart:
            markerMoved = grid.setStart(cell) || markerMoved;
            break;
        case DragMode::DragEnd:
            markerMoved = grid.setEnd(cell) || markerMoved;
            break;
        case DragMode::None:
            break;
        }

        if (x0 == x1 && y0 == y1) {
            break;
        }

        const int e2 = 2 * error;
        if (e2 >= dy) {
            error += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            error += dx;
            y0 += sy;
        }
    }
}

bool InputHandler::handle(const sf::Event& event,
                          const sf::RenderWindow& window,
                          Grid& grid,
                          const GridRenderer& renderer,
                          bool editingAllowed,
                          bool& markerMoved) {
    markerMoved = false;

    // Mouse button release cancels active dragging
    if (event.type == sf::Event::MouseButtonReleased) {
        if (event.mouseButton.button == sf::Mouse::Left ||
            event.mouseButton.button == sf::Mouse::Right) {
            reset();
        }
        return false;
    }

    // Mouse button press initiates dragging
    if (event.type == sf::Event::MouseButtonPressed) {
        if (!editingAllowed) {
            return false;
        }

        const auto clickedCell = renderer.screenToCell({event.mouseButton.x, event.mouseButton.y}, window);
        if (!clickedCell) {
            return false;
        }

        previousCoord_ = *clickedCell;
        hasPrevious_ = true;

        if (event.mouseButton.button == sf::Mouse::Right) {
            mode_ = DragMode::RightErase;
        } else if (event.mouseButton.button == sf::Mouse::Left) {
            const auto state = grid.get(*clickedCell);
            if (state == NodeState::Start) {
                mode_ = DragMode::DragStart;
            } else if (state == NodeState::End) {
                mode_ = DragMode::DragEnd;
            } else if (state == NodeState::Wall) {
                mode_ = DragMode::EraseWall;
            } else {
                mode_ = DragMode::DrawWall;
            }
        } else {
            return false;
        }

        applyLine(*clickedCell, *clickedCell, grid, markerMoved);
        return true;
    }

    // Mouse movement paints or drags markers along the path
    if (event.type == sf::Event::MouseMoved && mode_ != DragMode::None && editingAllowed) {
        const auto currentCell = renderer.screenToCell({event.mouseMove.x, event.mouseMove.y}, window);
        if (!currentCell) {
            return false;
        }

        if (!hasPrevious_) {
            previousCoord_ = *currentCell;
            hasPrevious_ = true;
        }

        applyLine(previousCoord_, *currentCell, grid, markerMoved);
        previousCoord_ = *currentCell;
        return true;
    }

    return false;
}

} // namespace pfv

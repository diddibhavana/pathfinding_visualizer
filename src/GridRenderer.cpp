#include "pfv/GridRenderer.hpp"

#include <algorithm>
#include <cmath>

namespace pfv {

GridRenderer::GridRenderer(int rows, int cols)
    : rows_(rows),
      cols_(cols),
      cells_(sf::Quads, static_cast<std::size_t>(rows * cols * 4)) {
    rebuildGeometry();
}

sf::Color GridRenderer::colorFor(NodeState state) const {
    switch (state) {
    case NodeState::Wall:
        return palette::wall;
    case NodeState::Start:
        return palette::start;
    case NodeState::End:
        return palette::end;
    case NodeState::Open:
        return palette::open;
    case NodeState::Closed:
        return palette::closed;
    case NodeState::Path:
        return palette::path;
    case NodeState::Unvisited:
        return palette::cell;
    }
    return palette::cell;
}

void GridRenderer::resize(const sf::RenderWindow& window) {
    const auto size = window.getSize();
    const float availableW = static_cast<float>(size.x);
    const float availableH = std::max(1.0F, static_cast<float>(size.y) - top_);

    // Keep square grid cells and center horizontally
    const float scale = std::min(availableW / static_cast<float>(cols_),
                                 availableH / static_cast<float>(rows_));
    cellW_ = scale;
    cellH_ = scale;
    left_ = (availableW - cellW_ * static_cast<float>(cols_)) * 0.5F;

    rebuildGeometry();
}

void GridRenderer::rebuildGeometry() {
    // Generate vertex positions for each cell quad
    for (int r = 0; r < rows_; ++r) {
        for (int c = 0; c < cols_; ++c) {
            const std::size_t baseIdx = static_cast<std::size_t>((r * cols_ + c) * 4);
            const float x = left_ + static_cast<float>(c) * cellW_;
            const float y = top_ + static_cast<float>(r) * cellH_;

            cells_[baseIdx + 0].position = sf::Vector2f(x, y);
            cells_[baseIdx + 1].position = sf::Vector2f(x + cellW_, y);
            cells_[baseIdx + 2].position = sf::Vector2f(x + cellW_, y + cellH_);
            cells_[baseIdx + 3].position = sf::Vector2f(x, y + cellH_);
        }
    }

    // Rebuild grid border lines
    lines_.clear();
    lines_.setPrimitiveType(sf::Lines);

    // Vertical grid lines
    const float gridBottom = top_ + static_cast<float>(rows_) * cellH_;
    for (int c = 0; c <= cols_; ++c) {
        const float x = left_ + static_cast<float>(c) * cellW_;
        lines_.append(sf::Vertex(sf::Vector2f(x, top_), palette::gridLine));
        lines_.append(sf::Vertex(sf::Vector2f(x, gridBottom), palette::gridLine));
    }

    // Horizontal grid lines
    const float gridRight = left_ + static_cast<float>(cols_) * cellW_;
    for (int r = 0; r <= rows_; ++r) {
        const float y = top_ + static_cast<float>(r) * cellH_;
        lines_.append(sf::Vertex(sf::Vector2f(left_, y), palette::gridLine));
        lines_.append(sf::Vertex(sf::Vector2f(gridRight, y), palette::gridLine));
    }
}

void GridRenderer::rebuildAll(const Grid& grid) {
    for (int r = 0; r < rows_; ++r) {
        for (int c = 0; c < cols_; ++c) {
            const Coord coord{r, c};
            const sf::Color color = colorFor(grid.get(coord));
            const std::size_t baseIdx = static_cast<std::size_t>((r * cols_ + c) * 4);

            for (std::size_t k = 0; k < 4; ++k) {
                cells_[baseIdx + k].color = color;
            }
        }
    }
    dirty_.clear();
}

void GridRenderer::markDirty(Coord c) {
    if (c.row >= 0 && c.row < rows_ && c.col >= 0 && c.col < cols_) {
        dirty_.push_back(c);
    }
}

void GridRenderer::flush(const Grid& grid) {
    for (const Coord coord : dirty_) {
        const sf::Color color = colorFor(grid.get(coord));
        const std::size_t baseIdx = static_cast<std::size_t>((coord.row * cols_ + coord.col) * 4);

        for (std::size_t k = 0; k < 4; ++k) {
            cells_[baseIdx + k].color = color;
        }
    }
    dirty_.clear();
}

void GridRenderer::draw(sf::RenderWindow& window) const {
    window.draw(cells_);
    window.draw(lines_);
}

std::optional<Coord> GridRenderer::screenToCell(sf::Vector2i point, const sf::RenderWindow& window) const {
    const sf::Vector2f worldPos = window.mapPixelToCoords(point);
    const int col = static_cast<int>(std::floor((worldPos.x - left_) / cellW_));
    const int row = static_cast<int>(std::floor((worldPos.y - top_) / cellH_));

    if (row < 0 || row >= rows_ || col < 0 || col >= cols_) {
        return std::nullopt;
    }

    return Coord{row, col};
}

} // namespace pfv

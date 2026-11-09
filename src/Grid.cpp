#include "pfv/Grid.hpp"

#include <algorithm>
#include <stdexcept>

namespace pfv {

Grid::Grid(int rows, int cols)
    : rows_(rows),
      cols_(cols),
      start_{rows / 2, cols / 4},
      end_{rows / 2, (3 * cols) / 4},
      cells_(static_cast<std::size_t>(rows > 0 && cols > 0 ? rows * cols : 0), NodeState::Unvisited) {
    if (rows <= 0 || cols < 2) {
        throw std::invalid_argument("Grid must have positive rows and at least two columns");
    }
    reset();
}

bool Grid::inBounds(Coord c) const noexcept {
    return c.row >= 0 && c.col >= 0 && c.row < rows_ && c.col < cols_;
}

int Grid::index(Coord c) const {
    if (!inBounds(c)) {
        throw std::out_of_range("Grid coordinate out of bounds");
    }
    return c.row * cols_ + c.col;
}

NodeState Grid::get(Coord c) const {
    return cells_.at(static_cast<std::size_t>(index(c)));
}

void Grid::set(Coord c, NodeState state) {
    cells_.at(static_cast<std::size_t>(index(c))) = state;
}

std::vector<Coord> Grid::neighbors(Coord c) const {
    // 4-connected grid: North, East, South, West
    static constexpr Coord kOffsets[] = {
        {-1,  0},  // Up
        { 0,  1},  // Right
        { 1,  0},  // Down
        { 0, -1}   // Left
    };

    std::vector<Coord> result;
    result.reserve(4);

    for (const Coord& offset : kOffsets) {
        const Coord next{c.row + offset.row, c.col + offset.col};
        if (inBounds(next) && get(next) != NodeState::Wall) {
            result.push_back(next);
        }
    }

    return result;
}

bool Grid::setStart(Coord c) {
    if (!inBounds(c) || c == end_) {
        return false;
    }
    set(start_, NodeState::Unvisited);
    start_ = c;
    set(c, NodeState::Start);
    return true;
}

bool Grid::setEnd(Coord c) {
    if (!inBounds(c) || c == start_) {
        return false;
    }
    set(end_, NodeState::Unvisited);
    end_ = c;
    set(c, NodeState::End);
    return true;
}

bool Grid::setWall(Coord c, bool wall) {
    if (!inBounds(c) || c == start_ || c == end_) {
        return false;
    }
    set(c, wall ? NodeState::Wall : NodeState::Unvisited);
    return true;
}

void Grid::clearPath() {
    for (auto& state : cells_) {
        if (state == NodeState::Open || state == NodeState::Closed || state == NodeState::Path) {
            state = NodeState::Unvisited;
        }
    }
    set(start_, NodeState::Start);
    set(end_, NodeState::End);
}

void Grid::clearWalls() {
    for (auto& state : cells_) {
        if (state == NodeState::Wall) {
            state = NodeState::Unvisited;
        }
    }
}

void Grid::reset() {
    std::fill(cells_.begin(), cells_.end(), NodeState::Unvisited);
    start_ = {rows_ / 2, cols_ / 4};
    end_ = {rows_ / 2, (3 * cols_) / 4};
    set(start_, NodeState::Start);
    set(end_, NodeState::End);
}

} // namespace pfv

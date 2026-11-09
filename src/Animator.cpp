#include "pfv/Animator.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

namespace pfv {

void Animator::load(SearchResult result) {
    result_ = std::move(result);
    playbackIndex_ = 0;
    accumulator_ = 0.0F;
    playing_ = false;
}

bool Animator::applyNext(Grid& grid, std::vector<Coord>& dirtyCells) {
    if (isFinished()) {
        return false;
    }

    const auto& event = result_.events[playbackIndex_++];
    grid.set(event.cell, event.newState);
    dirtyCells.push_back(event.cell);

    if (isFinished()) {
        playing_ = false;
    }

    return true;
}

bool Animator::stepOnce(Grid& grid, std::vector<Coord>& dirtyCells) {
    playing_ = false;
    accumulator_ = 0.0F;
    return applyNext(grid, dirtyCells);
}

void Animator::update(float dt, Grid& grid, std::vector<Coord>& dirtyCells) {
    if (!playing_ || isFinished()) {
        return;
    }

    // Clamp dt to avoid huge step jumps during window dragging or stutters
    const float clampedDt = std::max(0.0F, std::min(dt, 0.1F));
    accumulator_ += clampedDt * static_cast<float>(speed());

    const auto stepsToExecute = static_cast<std::size_t>(std::floor(accumulator_));
    accumulator_ -= static_cast<float>(stepsToExecute);

    for (std::size_t i = 0; i < stepsToExecute && playing_; ++i) {
        applyNext(grid, dirtyCells);
    }
}

float Animator::progress() const noexcept {
    if (result_.events.empty()) {
        return 1.0F;
    }
    return static_cast<float>(playbackIndex_) / static_cast<float>(result_.events.size());
}

void Animator::faster() noexcept {
    if (speedIndex_ + 1 < speeds_.size()) {
        ++speedIndex_;
    }
}

void Animator::slower() noexcept {
    if (speedIndex_ > 0) {
        --speedIndex_;
    }
}

} // namespace pfv

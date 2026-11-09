#pragma once

#include "pfv/Grid.hpp"

#include <array>
#include <vector>

namespace pfv {

/// Handles frame-independent replay of pathfinding search events.
class Animator {
public:
    void load(SearchResult result);

    void play() noexcept {
        if (!isFinished()) {
            playing_ = true;
        }
    }

    void pause() noexcept {
        playing_ = false;
    }

    void toggle() noexcept {
        if (playing_) {
            pause();
        } else {
            play();
        }
    }

    /// Advance by exactly one search event (useful when paused).
    bool stepOnce(Grid& grid, std::vector<Coord>& dirtyCells);

    /// Advance playback based on elapsed delta time dt.
    void update(float dt, Grid& grid, std::vector<Coord>& dirtyCells);

    bool isFinished() const noexcept {
        return playbackIndex_ >= result_.events.size();
    }

    float progress() const noexcept;

    // Speed controls
    int speed() const noexcept {
        return speeds_[speedIndex_];
    }
    void faster() noexcept;
    void slower() noexcept;

    bool playing() const noexcept {
        return playing_;
    }

    const SearchResult& result() const noexcept {
        return result_;
    }

private:
    bool applyNext(Grid& grid, std::vector<Coord>& dirtyCells);

    // Discrete speed tiers in steps per second
    static constexpr std::array<int, 8> speeds_{{10, 30, 60, 120, 300, 600, 1500, 5000}};

    SearchResult result_;
    std::size_t playbackIndex_{0};
    std::size_t speedIndex_{3};  // Default: 120 steps/s
    float accumulator_{0.0F};
    bool playing_{false};
};

} // namespace pfv

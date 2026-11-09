#pragma once

#include "pfv/Animator.hpp"
#include "pfv/GridRenderer.hpp"

#include <SFML/Graphics.hpp>

namespace pfv {

/// Head-Up Display: renders top status bar, metrics, color legend, and help overlay.
class Hud {
public:
    Hud();

    void toggleHelp() noexcept {
        help_ = !help_;
    }

    void draw(sf::RenderWindow& window,
              const char* algorithm,
              const Animator& animator,
              const char* state,
              bool hasResult) const;

private:
    sf::Font font_;
    bool fontLoaded_{false};
    bool help_{true};
};

} // namespace pfv

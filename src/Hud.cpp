#include "pfv/Hud.hpp"

#include <array>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <utility>

namespace pfv {

Hud::Hud() {
    // Search order for fonts: local asset folder, then common OS system fonts
    const std::array<const char*, 4> candidatePaths{{
        "assets/font.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
        "/System/Library/Fonts/Supplemental/Arial.ttf"
    }};

    for (const char* path : candidatePaths) {
        if (font_.loadFromFile(path)) {
            fontLoaded_ = true;
            break;
        }
    }

    if (!fontLoaded_) {
        std::cerr << "Warning: No suitable font could be loaded. HUD text will be disabled.\n"
                  << "Place a TrueType font at 'assets/font.ttf' to enable text overlay.\n";
    }
}

void Hud::draw(sf::RenderWindow& window,
              const char* algorithm,
              const Animator& animator,
              const char* state,
              bool hasResult) const {
    const float windowWidth = static_cast<float>(window.getSize().x);
    const float windowHeight = static_cast<float>(window.getSize().y);

    // Top HUD background bar
    sf::RectangleShape topBar(sf::Vector2f(windowWidth, 56.0F));
    topBar.setFillColor(palette::background);
    window.draw(topBar);

    if (!fontLoaded_) {
        return;
    }

    // Top primary status line
    std::ostringstream statusStream;
    statusStream << algorithm << "   |   " << animator.speed() << " steps/s   |   " << state;

    if (hasResult) {
        const auto& res = animator.result();
        statusStream << "   |   Visited: " << res.visitedCount
                     << "   |   Path length: " << res.pathLength
                     << "   |   Compute: " << std::fixed << std::setprecision(2) << res.computeMs << " ms";
        if (!res.found) {
            statusStream << " (No path found)";
        }
    }

    sf::Text statusText(statusStream.str(), font_, 16);
    statusText.setFillColor(sf::Color(235, 238, 242));
    statusText.setPosition(12.0F, 8.0F);
    window.draw(statusText);

    // Color swatches & legend
    const std::array<std::pair<const char*, sf::Color>, 6> legendItems{{
        {"Start", palette::start},
        {"End", palette::end},
        {"Wall", palette::wall},
        {"Frontier", palette::open},
        {"Visited", palette::closed},
        {"Path", palette::path}
    }};

    float legendX = 12.0F;
    const float legendY = 34.0F;

    for (const auto& [label, color] : legendItems) {
        sf::RectangleShape swatch(sf::Vector2f(10.0F, 10.0F));
        swatch.setFillColor(color);
        swatch.setPosition(legendX, legendY);
        window.draw(swatch);

        sf::Text swatchLabel(label, font_, 11);
        swatchLabel.setFillColor(sf::Color(210, 214, 220));
        swatchLabel.setPosition(legendX + 14.0F, legendY - 3.0F);
        window.draw(swatchLabel);

        legendX += 76.0F;
    }

    // Bottom help overlay
    if (help_) {
        sf::Text helpText(
            "Space: Run/Pause  |  1-4: Select Algo  |  Left Drag: Draw/Move  |  Right: Erase  |  "
            "C: Clear Path  |  W: Clear Walls  |  R: Reset  |  M: Maze  |  Up/Down: Speed  |  Right: Step  |  H: Help",
            font_,
            12
        );
        helpText.setFillColor(sf::Color(180, 185, 194));
        helpText.setPosition(12.0F, windowHeight - 22.0F);
        window.draw(helpText);
    }
}

} // namespace pfv

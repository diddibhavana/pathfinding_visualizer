#pragma once

#include "pfv/Animator.hpp"
#include "pfv/GridRenderer.hpp"
#include "pfv/Hud.hpp"
#include "pfv/InputHandler.hpp"
#include "pfv/Pathfinder.hpp"

#include <SFML/Graphics.hpp>

#include <memory>

namespace pfv {

/// Main application class managing window lifecycle, event loop, and subsystems.
class App {
public:
    App();

    /// Runs the main application loop. Returns exit status code.
    int run();

private:
    enum class State {
        Idle,
        Running,
        Paused,
        Finished
    };

    void pollEvents();
    void update(float dt);
    void render();
    void handleKey(sf::Keyboard::Key key);

    void beginRun(bool paused);
    void clearToIdle();
    void recomputeInstant();
    void randomize();

    const char* stateName() const noexcept;

    sf::RenderWindow window_;
    Grid grid_;
    Animator animator_;
    GridRenderer renderer_;
    Hud hud_;
    InputHandler input_;

    std::unique_ptr<IPathfinder> algorithm_;
    AlgorithmType algorithmType_{AlgorithmType::Dijkstra};
    State state_{State::Idle};
    bool hasResult_{false};
};

} // namespace pfv

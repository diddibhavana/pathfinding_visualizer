#include "pfv/App.hpp"

#include <random>
#include <utility>
#include <vector>

namespace pfv {

App::App()
    : window_(sf::VideoMode(1280, 800), "Pathfinding Visualizer", sf::Style::Default),
      grid_(),
      animator_(),
      renderer_(grid_.rows(), grid_.cols()),
      hud_(),
      input_(),
      algorithm_(makePathfinder(algorithmType_)) {
    window_.setVerticalSyncEnabled(true);
    renderer_.resize(window_);
    renderer_.rebuildAll(grid_);
}

int App::run() {
    sf::Clock clock;

    while (window_.isOpen()) {
        pollEvents();
        update(clock.restart().asSeconds());
        render();
    }

    return 0;
}

const char* App::stateName() const noexcept {
    switch (state_) {
    case State::Idle:
        return "Idle";
    case State::Running:
        return "Running";
    case State::Paused:
        return "Paused";
    case State::Finished:
        return "Finished";
    }
    return "Idle";
}

void App::clearToIdle() {
    state_ = State::Idle;
    hasResult_ = false;
    animator_.pause();
    input_.reset();
}

void App::beginRun(bool paused) {
    grid_.clearPath();
    renderer_.rebuildAll(grid_);

    animator_.load(algorithm_->solve(grid_));
    hasResult_ = true;

    if (paused) {
        state_ = State::Paused;
    } else {
        state_ = State::Running;
        animator_.play();
    }
}

void App::recomputeInstant() {
    grid_.clearPath();

    SearchResult result = algorithm_->solve(grid_);
    hasResult_ = true;

    for (const auto& event : result.events) {
        grid_.set(event.cell, event.newState);
    }

    animator_.load(std::move(result));
    renderer_.rebuildAll(grid_);
    state_ = State::Finished;
}

void App::randomize() {
    // Fixed seed gives reproducible obstacle patterns
    std::mt19937 rng(0x5EEDU);
    std::uniform_real_distribution<float> distribution(0.0F, 1.0F);

    grid_.clearWalls();
    grid_.clearPath();

    constexpr float kObstacleDensity = 0.28F;

    for (int r = 0; r < grid_.rows(); ++r) {
        for (int c = 0; c < grid_.cols(); ++c) {
            const Coord cell{r, c};
            if (cell != grid_.start() && cell != grid_.end() && distribution(rng) < kObstacleDensity) {
                grid_.setWall(cell, true);
            }
        }
    }

    clearToIdle();
    renderer_.rebuildAll(grid_);
}

void App::handleKey(sf::Keyboard::Key key) {
    if (key == sf::Keyboard::Escape) {
        window_.close();
        return;
    }

    if (key == sf::Keyboard::H) {
        hud_.toggleHelp();
        return;
    }

    if (key == sf::Keyboard::Up) {
        animator_.faster();
        return;
    }

    if (key == sf::Keyboard::Down) {
        animator_.slower();
        return;
    }

    // Spacebar toggles Run / Pause / Resume
    if (key == sf::Keyboard::Space) {
        if (state_ == State::Idle || state_ == State::Finished) {
            beginRun(false);
        } else if (state_ == State::Running) {
            state_ = State::Paused;
            animator_.pause();
        } else {
            state_ = State::Running;
            animator_.play();
        }
        return;
    }

    // Right arrow steps forward once
    if (key == sf::Keyboard::Right) {
        if (state_ == State::Idle || state_ == State::Finished) {
            beginRun(true);
        }
        if (state_ == State::Paused) {
            std::vector<Coord> dirtyCells;
            animator_.stepOnce(grid_, dirtyCells);
            for (const Coord cell : dirtyCells) {
                renderer_.markDirty(cell);
            }
            renderer_.flush(grid_);

            if (animator_.isFinished()) {
                state_ = State::Finished;
            }
        }
        return;
    }

    // Algorithm selection and grid operations
    AlgorithmType selectedType = algorithmType_;
    bool algorithmChanged = false;

    switch (key) {
    case sf::Keyboard::Num1:
    case sf::Keyboard::Numpad1:
        selectedType = AlgorithmType::Dijkstra;
        algorithmChanged = true;
        break;

    case sf::Keyboard::Num2:
    case sf::Keyboard::Numpad2:
        selectedType = AlgorithmType::AStar;
        algorithmChanged = true;
        break;

    case sf::Keyboard::Num3:
    case sf::Keyboard::Numpad3:
        selectedType = AlgorithmType::BFS;
        algorithmChanged = true;
        break;

    case sf::Keyboard::Num4:
    case sf::Keyboard::Numpad4:
        selectedType = AlgorithmType::DFS;
        algorithmChanged = true;
        break;

    case sf::Keyboard::C:
        grid_.clearPath();
        clearToIdle();
        renderer_.rebuildAll(grid_);
        return;

    case sf::Keyboard::W:
        grid_.clearWalls();
        grid_.clearPath();
        clearToIdle();
        renderer_.rebuildAll(grid_);
        return;

    case sf::Keyboard::R:
        grid_.reset();
        clearToIdle();
        renderer_.rebuildAll(grid_);
        return;

    case sf::Keyboard::M:
        randomize();
        return;

    default:
        break;
    }

    if (algorithmChanged && selectedType != algorithmType_) {
        algorithmType_ = selectedType;
        algorithm_ = makePathfinder(algorithmType_);
        grid_.clearPath();
        clearToIdle();
        renderer_.rebuildAll(grid_);
    }
}

void App::pollEvents() {
    sf::Event event{};

    while (window_.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window_.close();
        } else if (event.type == sf::Event::Resized) {
            const sf::FloatRect viewRect(0.0F, 0.0F,
                                         static_cast<float>(event.size.width),
                                         static_cast<float>(event.size.height));
            window_.setView(sf::View(viewRect));
            renderer_.resize(window_);
            renderer_.rebuildAll(grid_);
        } else if (event.type == sf::Event::KeyPressed) {
            handleKey(event.key.code);
        } else {
            bool markerMoved = false;
            const bool edited = input_.handle(event, window_, grid_, renderer_,
                                              state_ != State::Running, markerMoved);
            if (edited) {
                // If user dragged start/end marker after run finished, instantly update path
                if (state_ == State::Finished && markerMoved) {
                    recomputeInstant();
                } else {
                    clearToIdle();
                    renderer_.rebuildAll(grid_);
                }
            }
        }
    }
}

void App::update(float dt) {
    std::vector<Coord> dirtyCells;
    animator_.update(dt, grid_, dirtyCells);

    for (const Coord cell : dirtyCells) {
        renderer_.markDirty(cell);
    }
    renderer_.flush(grid_);

    if (state_ == State::Running && animator_.isFinished()) {
        state_ = State::Finished;
    }
}

void App::render() {
    window_.clear(palette::background);
    renderer_.draw(window_);
    hud_.draw(window_, algorithm_->name(), animator_, stateName(), hasResult_);
    window_.display();
}

} // namespace pfv

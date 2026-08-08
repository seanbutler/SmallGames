#pragma once
#include "iscreen.hpp"
#include "game.hpp"

class GameScreen : public IScreen {
public:
    void handleEvent(const SDL_Event& event) override;
    void update(float dt) override;
    void draw() const override;
    ScreenSignal signal() const override;

protected:
    // game_ is uninitialised when this constructor runs (derived member not yet
    // constructed). Only store the reference here — never dereference in the ctor.
    explicit GameScreen(Game& game);

private:
    Game&        game_;
    ScreenSignal signal_{ScreenSignal::None};
};

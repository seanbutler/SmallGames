#pragma once
#include "entity.hpp"
#include "message.hpp"
#include "pong_constants.hpp"

class Ball : public Entity {
public:
    explicit Ball(MessageBus& bus);

    void update(float dt) override;
    void draw(SDL_Renderer* renderer) const override;

    void serve(bool moveRight);

    Vec2  vel{};
    float speed{BALL_SPEED_BASE};

private:
    MessageBus& bus_;
};

#include "file.hpp"

using namespace godot;

void BoucySprite::_bind_methods() {

}

BoucySprite::BoucySprite() {
    left_bounds = 0;
    right_bounds = 1000;
    top_bounds = 0;
    bottom_bounds = 800;
    velocity = Vector2(0,0);
}

BoucySprite::~BoucySprite() {

}

void BoucySprite::_process(double delta) {
    velocity.x += speed * delta;

    Vector2 pos = get_position();
    pos += velocity;
    set_position(pos);
}

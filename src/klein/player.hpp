#pragma once

#include <SFML/System/Time.hpp>
#include <entt/entity/fwd.hpp>

#include "klein/input.hpp"

namespace klein::player {
    struct Player {
        float move_vel = 300.0f; // units/sec
        float jump_vel = 250.0f;// units/sec
    };

    void update_player_movement(
        entt::registry& registry,
        const input::InputState &input
    );
}

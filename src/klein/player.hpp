#pragma once

#include <SFML/System/Time.hpp>
#include <entt/entity/fwd.hpp>

#include "klein/input.hpp"

namespace klein::player {
    struct Player {
        float move_speed = 200.0f; // units/sec
    };

    void update_player_movement(
        entt::registry& registry,
        const input::InputState &input,
        const sf::Time &dt
    );
}

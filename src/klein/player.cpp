#include "klein/player.hpp"

#include <SFML/Graphics/Transform.hpp>
#include <entt/entt.hpp>

#include "klein/physics.hpp"

namespace klein::player {
    void update_player_movement(
        entt::registry& registry,
        const input::InputState &input
    ) {
        auto view = registry.view<Player, physics::Velocity, physics::KinematicBody>();
        for (auto [entity, player, vel, body]: view.each()) {
            vel.v.x = input.movement.x * player.move_vel;
            if (input.jump && vel.v.y == 0.0f && body.on_ground)
                vel.v.y = -player.jump_vel;
        }
    }
}

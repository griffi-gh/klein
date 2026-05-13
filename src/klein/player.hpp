#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/Graphics/Transform.hpp>
#include <SFML/System/Vector2.hpp>
#include <entt/entity/fwd.hpp>

#include "klein/animation/animation_loader.hpp"
#include "klein/camera.hpp"
#include "klein/input.hpp"

namespace klein::player {
    constexpr sf::Vector2f PLAYER_COLLIDER_SIZE = sf::Vector2f(40.f, 68.f);

    struct Player {
        float move_vel = 300.0f; // units/sec
        float jump_vel = 250.0f; // units/sec
        float min_jump_vel = 100.0f;
    };

    struct RespawnPoint {
        sf::Vector2f position;
    };

    entt::entity create_player_entity(
        entt::registry &registry,
        sf::Texture animation_texture,
        animation::LoadedAnimations animations,
        sf::Vector2f translate = {}
    );

    void update_player_movement(
        entt::registry& registry,
        const input::InputState &input
    );

    void update_player_animations(
        entt::registry& registry,
        const input::InputState &input
    );

    void detect_player_portal_cross(
        entt::registry& registry,
        const sf::Time &dt,
        camera::Camera2d &camera
    );

    void debug_draw_player_hitbox(
        const entt::registry& registry,
        sf::RenderTarget &target
    );

    struct Dead {
        sf::Clock when {};
        float respawn_time = 3.f; //**< in secs */
    };

    void detect_player_spikes(entt::registry& registry);

    /// Check for respawnable player that are Dead and respawn em
    void handle_respawn(entt::registry& registry);
}

#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <entt/entity/fwd.hpp>

namespace klein::physics {
    struct KinematicBody {
        sf::Vector2f size;
        bool on_ground = false;

        inline KinematicBody(sf::Vector2f size): size(size) {};
    };

    struct Velocity {
        sf::Vector2f v{0.f, 0.f};
    };

    void update_gravity(
        entt::registry& registry,
        const sf::Time& dt,
        float gravity = 32.0 * 9.81f
    );

    /// Step physics forward by dt
    void step_physics(
        entt::registry& registry,
        const sf::Time& dt
    );
}

#include "klein/player.hpp"

#include <SFML/Graphics/Transform.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <entt/entt.hpp>

#include "klein/physics.hpp"
#include "klein/drawable.hpp"

namespace klein::player {
    entt::entity create_player_entity(entt::registry &registry, sf::Vector2f translate) {
        auto entity = registry.create();

        sf::RectangleShape player_drawable({30., 60.});
        player_drawable.setOrigin(player_drawable.getSize().componentWiseMul({ 0.5, 0.5 }));
        registry.emplace<drawable::drawable_ptr>(entity,
            std::make_unique<sf::RectangleShape>(std::move(player_drawable)));
        registry.emplace<player::Player>(entity);
        registry.emplace<sf::Transform>(entity, sf::Transform{}.translate(translate));
        registry.emplace<physics::KinematicBody>(entity, physics::KinematicBody {
            .size = player_drawable.getSize()
        });
        registry.emplace<physics::Velocity>(entity);

        return entity;
    }

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

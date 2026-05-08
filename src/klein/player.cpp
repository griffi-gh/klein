#include "klein/player.hpp"

#include <SFML/Graphics/Transform.hpp>
#include <entt/entt.hpp>

namespace klein::player {
    void update_player_movement(
        entt::registry& registry,
        const input::InputState &input,
        const sf::Time &dt
    ) {
        const float x = (input.right ? 1.0f : 0.0f) - (input.left ? 1.0f : 0.0f);
        const float y = (input.down ? 1.0f : 0.0f) - (input.up ? 1.0f : 0.0f);
        if (x != 0.0f || y != 0.0f) {
            auto view = registry.view<Player, sf::Transform>();
            for (auto entity : view) {
                auto& player = view.get<Player>(entity);
                auto& transform = view.get<sf::Transform>(entity);
                transform.translate({x * player.move_speed * dt.asSeconds(), y * player.move_speed * dt.asSeconds()});
            }
        }
    }
}

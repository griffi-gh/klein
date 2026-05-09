#include "klein/physics.hpp"

#include <SFML/Graphics/Transform.hpp>
#include <SFML/System/Vector2.hpp>
#include <entt/entt.hpp>

#include "klein/tilemap/tilemap.hpp"

namespace klein::physics {
    static bool is_solid_at(entt::registry& registry, sf::Vector2i tile) {
        for (auto [map_e, map]: registry.view<tilemap::TileMap>().each()) {
            (void)map_e;
            for (const auto& layer : map.layers) {
                if (!layer.collider) continue;
                if (layer.get(tile)) return true;
            }
        }
        return false;
    }

    void update_gravity(
        entt::registry& registry,
        const sf::Time& dt,
        const float gravity
    ) {
        const auto view = registry.view<Body, Velocity>();
        for (const auto &[entity, vel]: view.each()) {
            vel.v.y += gravity * dt.asSeconds();
        }
    }

    void step_physics(
        entt::registry& registry,
        const sf::Time& dt
    ) {
        const auto view = registry.view<Body, Velocity, sf::Transform>();
        for (const auto &[entity, vel, trans]: view.each()) {
            const sf::Vector2f pos = trans.transformPoint({});

            sf::Vector2f advance {};

            const sf::Vector2f next_x = pos + sf::Vector2f(0, vel.v.x * dt.asSeconds());
            const sf::Vector2i next_x_tile = sf::Vector2i(next_x.componentWiseDiv(tilemap::TILE_SCREEN_SIZE));
            if (is_solid_at(registry, next_x_tile)) {
                vel.v.x = 0.f;
            } else {
                advance.x = next_x.x - pos.x;
            }

            const sf::Vector2f next_y = pos + sf::Vector2f(0, vel.v.y * dt.asSeconds());
            const sf::Vector2i next_y_tile = sf::Vector2i(next_x.componentWiseDiv(tilemap::TILE_SCREEN_SIZE));
            if (is_solid_at(registry, next_y_tile)) {
                vel.v.y = 0.f;
            } else {
                advance.y = next_y.y - pos.y;
            }

            trans = trans.translate(advance);
        }
    }
}

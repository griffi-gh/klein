#include "klein/physics.hpp"

#include <SFML/Graphics/Transform.hpp>
#include <SFML/System/Vector2.hpp>
#include <entt/entt.hpp>

#include "klein/tilemap/tilemap.hpp"

constexpr float EPSILON = 0.001f;

namespace klein::physics {
    static bool check_point_tilespace(
        const entt::registry& registry,
        const sf::Vector2i tile
    ) {
        for (const auto &[_, map]: registry.view<const tilemap::TileMap>().each()) {
            for (const auto& layer: map.layers) {
                if (!layer.collider) continue;
                if (layer.get(tile)) return true;
            }
        }
        return false;
    }


    static std::optional<sf::Vector2i> check_row_worldspace(
        const entt::registry& registry,
        const float y,
        const float x_from,
        const float x_to
    ) {
        const int tile_y = std::floor(y / tilemap::TILE_SCREEN_SIZE.y);
        const int tile_x0 = std::floor(x_from / tilemap::TILE_SCREEN_SIZE.x);
        const int tile_x1 = std::floor((x_to - EPSILON) / tilemap::TILE_SCREEN_SIZE.x);
        for (int tile_x = tile_x0; tile_x <= tile_x1; ++tile_x)
            if (check_point_tilespace(registry, {tile_x, tile_y}))
                return sf::Vector2i { tile_x, tile_y };
        return std::nullopt;
    }

    static std::optional<sf::Vector2i> check_col_worldspace(
        const entt::registry& registry,
        const float x,
        const float y_from,
        const float y_to
    ) {
        const int tile_x = std::floor(x / tilemap::TILE_SCREEN_SIZE.x);
        const int tile_y0 = std::floor(y_from / tilemap::TILE_SCREEN_SIZE.y);
        const int tile_y1 = std::floor((y_to - EPSILON) / tilemap::TILE_SCREEN_SIZE.y);
        for (int tile_y = tile_y0; tile_y <= tile_y1; ++tile_y)
            if (check_point_tilespace(registry, {tile_x, tile_y}))
                return sf::Vector2i { tile_x, tile_y };
        return std::nullopt;
    }

    void update_gravity(
        entt::registry& registry,
        const sf::Time& dt,
        const float gravity
    ) {
        const auto view = registry.view<Velocity, const KinematicBody>();
        for (const auto &[entity, vel, _]: view.each()) {
            vel.v.y += gravity * dt.asSeconds();
        }
    }

    void step_physics(
        entt::registry& registry,
        const sf::Time& dt
    ) {
        const auto view = registry.view<Velocity, sf::Transform, KinematicBody>();
        for (const auto &[entity, vel, trans, body]: view.each()) {
            sf::Vector2f advance {};

            const sf::Vector2f pos = trans.transformPoint({});

            body.on_ground = false;

            // vertical check
            //
            const float next_y = pos.y + vel.v.y * dt.asSeconds();
            if (vel.v.y > 0.f) {
                const float next_bottom = next_y + body.size.y * 0.5;
                const auto hit = check_row_worldspace(
                    registry,
                    next_bottom,
                    pos.x - body.size.x * 0.5,
                    pos.x + body.size.x * 0.5
                );
                if (hit.has_value()) {
                    const int tile_y = std::floor(next_bottom / tilemap::TILE_SCREEN_SIZE.y);
                    advance.y = tile_y * tilemap::TILE_SCREEN_SIZE.y - body.size.y * 0.5 - pos.y;
                    vel.v.y = 0.f;
                    body.on_ground = true;
                } else {
                    advance.y = next_y - pos.y;
                }
            } else if (vel.v.y < 0.f) {
                const float next_top = next_y - body.size.y * 0.5;
                const auto hit = check_row_worldspace(
                    registry,
                    next_top,
                    pos.x - body.size.x * 0.5,
                    pos.x + body.size.x * 0.5
                );
                if (hit.has_value()) {
                    const int tile_y = std::floor(next_top / tilemap::TILE_SCREEN_SIZE.y);
                    advance.y = (tile_y + 1) * tilemap::TILE_SCREEN_SIZE.y + body.size.y * 0.5 - pos.y;
                    vel.v.y = 0.f;
                } else {
                    advance.y = next_y - pos.y;
                }
            }

            // horizontal check
            //
            const float res_y = pos.y + advance.y;
            const float next_x = pos.x + vel.v.x * dt.asSeconds();
            if (vel.v.x > 0.f) {
                const float next_right = next_x + body.size.x * 0.5;
                const auto hit = check_col_worldspace(
                    registry,
                    next_right,
                    res_y - body.size.y * 0.5,
                    res_y + body.size.y * 0.5
                );
                if (hit.has_value()) {
                    const int tile_x = std::floor(next_right / tilemap::TILE_SCREEN_SIZE.x);
                    advance.x = tile_x * tilemap::TILE_SCREEN_SIZE.x - body.size.x * 0.5 - pos.x;
                    vel.v.x = 0.f;
                } else {
                    advance.x = next_x - pos.x;
                }
            } else if (vel.v.x < 0.f) {
                const float next_left = next_x - body.size.x * 0.5;
                const auto hit = check_col_worldspace(
                    registry,
                    next_left,
                    res_y - body.size.y * 0.5,
                    res_y + body.size.y * 0.5
                );
                if (hit.has_value()) {
                    const int tile_x = std::floor(next_left / tilemap::TILE_SCREEN_SIZE.x);
                    advance.x = (tile_x + 1) * tilemap::TILE_SCREEN_SIZE.x + body.size.x * 0.5 - pos.x;
                    vel.v.x = 0.f;
                } else {
                    advance.x = next_x - pos.x;
                }
            }

            trans = trans.translate(advance);
        }
    }
}

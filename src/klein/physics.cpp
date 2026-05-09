#include "klein/physics.hpp"

#include <SFML/Graphics/Transform.hpp>
#include <SFML/System/Vector2.hpp>
#include <entt/entt.hpp>

#include "klein/tilemap/tilemap.hpp"

namespace klein::physics {

    static bool check_point_tilespace(
        entt::registry& registry,
        sf::Vector2i tile
    ) {
        for (auto [_, map]: registry.view<tilemap::TileMap>().each()) {
            for (const auto& layer : map.layers) {
                if (!layer.collider) continue;
                if (layer.get(tile)) return true;
            }
        }
        return false;
    }

    constexpr float EPSILON = 0.001f;

    static bool check_row_worldspace(
        entt::registry& registry,
        float y,
        float x_from,
        float x_to
    ) {
        const int tile_y = std::floor(y / tilemap::TILE_SCREEN_SIZE.y);
        const int tile_x0 = std::floor(x_from / tilemap::TILE_SCREEN_SIZE.x);
        const int tile_x1 = std::floor((x_to - EPSILON) / tilemap::TILE_SCREEN_SIZE.x);
        for (int tx = tile_x0; tx <= tile_x1; ++tx)
            if (check_point_tilespace(registry, {tx, tile_y})) return true;
        return false;
    }

    static bool check_col_worldspace(
        entt::registry& registry,
        float x,
        float y_from,
        float y_to
    ) {
        const int tile_x = std::floor(x / tilemap::TILE_SCREEN_SIZE.x);
        const int tile_y0 = std::floor(y_from / tilemap::TILE_SCREEN_SIZE.y);
        const int tile_y1 = std::floor((y_to - EPSILON) / tilemap::TILE_SCREEN_SIZE.y);
        for (int ty = tile_y0; ty <= tile_y1; ++ty)
            if (check_point_tilespace(registry, {tile_x, ty})) return true;
        return false;
    }

    void update_gravity(
        entt::registry& registry,
        const sf::Time& dt,
        const float gravity
    ) {
        const auto view = registry.view<KinematicBody, Velocity>();
        for (const auto &[entity, _, vel]: view.each()) {
            vel.v.y += gravity * dt.asSeconds();
        }
    }

    void step_physics(
        entt::registry& registry,
        const sf::Time& dt
    ) {
        const auto view = registry.view<KinematicBody, Velocity, sf::Transform>();
        for (const auto &[entity, body, vel, trans]: view.each()) {
            sf::Vector2f advance {};

            const sf::Vector2f pos = trans.transformPoint({});

            body.on_ground = false;

            // vertical check
            //
            const float next_y = pos.y + vel.v.y * dt.asSeconds();
            if (vel.v.y > 0.f) {
                const float next_bottom = next_y + body.size.y * 0.5;
                const bool hit = check_row_worldspace(
                    registry,
                    next_bottom,
                    pos.x - body.size.x * 0.5,
                    pos.x + body.size.x * 0.5
                );
                if (hit) {
                    const int tile_y = std::floor(next_bottom / tilemap::TILE_SCREEN_SIZE.y);
                    advance.y = tile_y * tilemap::TILE_SCREEN_SIZE.y - body.size.y * 0.5 - pos.y;
                    vel.v.y = 0.f;
                    body.on_ground = true;
                } else {
                    advance.y = next_y - pos.y;
                }
            } else if (vel.v.y < 0.f) {
                const float next_top = next_y - body.size.y * 0.5;
                const bool hit = check_row_worldspace(
                    registry,
                    next_top,
                    pos.x - body.size.x * 0.5,
                    pos.x + body.size.x * 0.5
                );
                if (hit) {
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
                const bool hit = check_col_worldspace(
                    registry,
                    next_right,
                    res_y - body.size.y * 0.5,
                    res_y + body.size.y * 0.5
                );
                if (hit) {
                    const int tile_x = std::floor(next_right / tilemap::TILE_SCREEN_SIZE.x);
                    advance.x = tile_x * tilemap::TILE_SCREEN_SIZE.x - body.size.x * 0.5 - pos.x;
                    vel.v.x = 0.f;
                } else {
                    advance.x = next_x - pos.x;
                }
            } else if (vel.v.x < 0.f) {
                const float next_left = next_x - body.size.x * 0.5;
                const bool hit = check_col_worldspace(
                    registry,
                    next_left,
                    res_y - body.size.y * 0.5,
                    res_y + body.size.y * 0.5
                );
                if (hit) {
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

#pragma once

#include <cassert>
#include <cmath>
#include <optional>
#include <variant>
#include <SFML/System/Vector2.hpp>
#include <entt/entt.hpp>

namespace klein::view {
    struct ResultContinue {
        sf::Vector2f offset = {};
    };
    struct ResultBlock {};
    using StepResult = std::variant<ResultContinue, ResultBlock>;

    /// which axis the ray crossed to enter this tile
    ///
    enum class Side { XAxis, YAxis };

    struct Hit {
        sf::Vector2i tile;
        float distance;
        Side side;
    };

    constexpr float RAYCAST_MAX_DISTANCE_TILES = 50.0;

    /// Traces a ray through the tilemap(s), calling the callback for each step taken
    /// (implementation of the DDA algorithm)
    ///
    inline std::optional<Hit> raycast_tiles(
        const sf::Vector2f origin_tile, // (in tile coords, 1u = 1 tile)
        const sf::Vector2f direction,
        auto step_callback
    ) {
        assert(direction.x != 0.0f || direction.y != 0.0f);

        sf::Vector2f pos = origin_tile;

        sf::Vector2i tile(
            (int)std::floor(pos.x),
            (int)std::floor(pos.y)
        );
        sf::Vector2f delta(
            std::abs(1.0f / direction.x),
            std::abs(1.0f / direction.y)
        );

        sf::Vector2i step {};
        sf::Vector2f side {};
        auto recompute_sides = [&]() {
            step = {0, 0};
            side = {0, 0};

            if (direction.x < 0) {
                step.x = -1;
                side.x = (pos.x - tile.x) * delta.x;
            } else {
                step.x =  1;
                side.x = (tile.x + 1.0f - pos.x) * delta.x;
            }

            if (direction.y < 0) {
                step.y = -1;
                side.y = (pos.y - tile.y) * delta.y;
            } else {
                step.y =  1;
                side.y = (tile.y + 1.0f - pos.y) * delta.y;
            }
        };
        recompute_sides();

        float dist_accum = 0.0f;

        while(true) {
            Side crossed;
            float local_t;
            if (side.x < side.y) {
                local_t = side.x;
                crossed = Side::XAxis;
                tile.x += step.x;
                side.x += delta.x;
            } else {
                local_t = side.y;
                crossed = Side::YAxis;
                tile.y += step.y;
                side.y += delta.y;
            }

            const float dist_total = dist_accum + local_t;
            if (dist_total > RAYCAST_MAX_DISTANCE_TILES) return std::nullopt;

            StepResult res = step_callback(tile, dist_total);

            if (std::holds_alternative<ResultBlock>(res)) {
                return Hit{
                    .tile = tile,
                    .distance = dist_total,
                    .side = crossed,
                };
            } else if (
                auto* cont = std::get_if<ResultContinue>(&res);
                cont && (cont->offset.x != 0 || cont->offset.y != 0)
            ) {
                pos.x += direction.x * local_t + cont->offset.x;
                pos.y += direction.y * local_t + cont->offset.y;
                tile.x = (int)std::floor(pos.x);
                tile.y = (int)std::floor(pos.y);
                recompute_sides();
                dist_accum = dist_total;
            }

        }
    }

}

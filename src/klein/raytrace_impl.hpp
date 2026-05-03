#pragma once
#include "SFML/System/Vector2.hpp"
#include "entt/entt.hpp"
#include <optional>

namespace klein {

    /// which axis the ray crossed to enter this tile
    ///
    enum class Side { XAxis, YAxis };

    struct Hit {
        sf::Vector2i tile;
        float distance;
        Side side;
    };

    constexpr float MAX_DISTANCE = 256.0;

    /// Traces a ray through the tilemap(s), calling the callback for each step taken
    /// (implementation of the DDA algorithm)
    ///
    inline std::optional<Hit> raytrace(
        const sf::Vector2f origin_tile, // (in tile coords, 1u = 1 tile)
        const sf::Vector2f direction,
        auto step_callback
    ) {
        assert(direction.x != 0.0f || direction.y != 0.0f);

        sf::Vector2i tile(
            (int)std::floor(origin_tile.x),
            (int)std::floor(origin_tile.y)
        );
        sf::Vector2f delta(
            std::abs(1.0f / direction.x),
            std::abs(1.0f / direction.y)
        );

        sf::Vector2i step;
        sf::Vector2f side;
        if (direction.x < 0) { step.x = -1; side.x = (origin_tile.x - tile.x) * delta.x; }
        else                 { step.x =  1; side.x = (tile.x + 1.0f - origin_tile.x) * delta.x; }
        if (direction.y < 0) { step.y = -1; side.y = (origin_tile.y - tile.y) * delta.y; }
        else                 { step.y =  1; side.y = (tile.y + 1.0f - origin_tile.y) * delta.y; }

        while(true) {
            Side crossed;
            float distance;
            if (side.x < side.y) {
                distance = side.x;
                if (distance > MAX_DISTANCE) return std::nullopt;
                crossed = Side::XAxis;
                tile.x += step.x;
                side.x += delta.x;
            } else {
                distance = side.y;
                if (distance > MAX_DISTANCE) return std::nullopt;
                crossed = Side::YAxis;
                tile.y += step.y;
                side.y += delta.y;
            }

            if (step_callback(tile)) {
                return Hit {
                    .tile = tile,
                    .distance = distance,
                    .side = crossed,
                };
            }

        }
    }

}

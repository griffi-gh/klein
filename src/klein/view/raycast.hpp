#pragma once

#include <cstdint>
#include <SFML/System/Vector2.hpp>

namespace klein::view {
    /// which side the ray crossed to enter/exit this tile
    ///
    enum class TileFace: uint8_t {
        Top    = 0,
        Left   = 1,
        Right  = 2,
        Bottom = 3,
    };

    struct StepResult {
        sf::Vector2f offset {};
        bool block = false;
    };

    struct Hit {
        sf::Vector2i tile; /**< tile that was/is going to be hit */
        float distance; /**< total combined distance passed from the starting point */
        TileFace entry_face; /**< face of the tile that was passed to enter the tile */
        TileFace exit_face; /**< face of the tile that will be passed to exit the tile */
    };

    /// Maximum distance ray is allowed to pass before it stops
    /// (in tiles)
    ///
    constexpr float RAYCAST_MAX_DISTANCE_TILES = 64.0;
}

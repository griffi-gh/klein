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
        sf::Vector2i tile;
        float distance;
        TileFace entry_face;
        TileFace exit_face;
    };

    constexpr float RAYCAST_MAX_DISTANCE_TILES = 64.0;
}

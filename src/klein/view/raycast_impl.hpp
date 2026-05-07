#pragma once

#include <cassert>
#include <cmath>
#include <optional>
#include <SFML/System/Vector2.hpp>
#include <entt/entt.hpp>

namespace klein::view {
    struct StepResult {
        sf::Vector2f offset {};
        bool block = false;
    };

    /// which side the ray crossed to enter/exit this tile
    ///
    enum class TileFace: uint8_t {
        Top    = 0,
        Left   = 1,
        Right  = 2,
        Bottom = 3,
    };

    struct Hit {
        sf::Vector2i tile;
        float distance;
        TileFace entry_face;
        TileFace exit_face;
    };

    constexpr float RAYCAST_MAX_DISTANCE_TILES = 64.0;

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
            (int)std::floor(pos.x), (int)std::floor(pos.y));
        sf::Vector2f delta(
            std::abs(1.0f / direction.x), std::abs(1.0f / direction.y));

        sf::Vector2i step {};
        sf::Vector2f side {};
        const auto recompute_sides = [&]() {
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

        const auto get_entry_exit = [&]() -> std::tuple<TileFace, TileFace> {
            TileFace entry, exit;
            if (side.x < side.y) {
                entry = (step.x > 0) ? TileFace::Left : TileFace::Right;
                exit  = (step.x > 0) ? TileFace::Right : TileFace::Left;
            } else {
                entry = (step.y > 0) ? TileFace::Top : TileFace::Bottom;
                exit  = (step.y > 0) ? TileFace::Bottom : TileFace::Top;
            }
            return { entry, exit };
        };

        const auto get_potential_hit = [&](float distance) -> Hit {
            const auto [entry, exit] = get_entry_exit();
            return Hit {
                .tile = tile,
                .distance = distance,
                .entry_face = entry,
                .exit_face = exit,
            };
        };

        float dist_accum = 0.0f;
        const auto handle_result = [&](
            StepResult res,
            float local_t,
            float distance
        ) -> bool {
            if (res.offset.x != 0. || res.offset.y != 0.) {
                constexpr float NUDGE = 1e-5f; // (HACK: workaround hangs when ray is redirected and local_t == 0)
                pos.x += direction.x * (local_t + NUDGE) + res.offset.x;
                pos.y += direction.y * (local_t + NUDGE) + res.offset.y;
                tile.x = (int)std::floor(pos.x);
                tile.y = (int)std::floor(pos.y);
                recompute_sides();
                dist_accum = distance;
            }
            return res.block;
        };

        const auto maybe_hit = get_potential_hit(0.0f);
        const auto res = step_callback(maybe_hit);
        if (handle_result(res, 0.0f, 0.0f)) // XXX: local_t == 0 is bad
            return maybe_hit;

        while(true) {
            float local_t;
            if (side.x < side.y) {
                local_t = side.x;
                tile.x += step.x;
                side.x += delta.x;
            } else {
                local_t = side.y;
                tile.y += step.y;
                side.y += delta.y;
            }

            const float dist_total = dist_accum + local_t;
            if (dist_total > RAYCAST_MAX_DISTANCE_TILES)
                return std::nullopt;

            const auto maybe_hit = get_potential_hit(dist_total);
            const auto res = step_callback(maybe_hit);
            if (handle_result(res, local_t, dist_total))
                return maybe_hit;
        }
    }

}

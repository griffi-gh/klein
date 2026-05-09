#pragma once

#include <unordered_map>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Clock.hpp>
#include <entt/entt.hpp>

#include "klein/view/view_raycast.hpp"

namespace klein::view {
    // constexpr float TILE_RENDER_TICKRATE = 1. / 10.; // 10 fps
    constexpr unsigned int MAX_TILE_RESOLUTION = 4096;

    struct RenderedTile {
        bool active = false;
        bool dirty = true;
        sf::Vector2i current_min_aabb {};
        sf::Vector2i current_max_aabb {};
        sf::RenderTexture target {};
        uint8_t _debug_state = 0;
    };

    class ViewTilesState {
    private:
        std::unordered_map<ViewKey, RenderedTile, ViewKeyHash> offscreen_pool {};

    public:
        void render_views_offscreen(
            entt::registry& registry,
            const RaycastViewResponse& raycast
        );

        void compose_views(
            sf::RenderTarget &target,
            const RaycastViewResponse& raycast
        ) const;
    };
}

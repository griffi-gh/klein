#pragma once

#include <unordered_map>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Clock.hpp>
#include <entt/entt.hpp>

#include "klein/view/view_raycast.hpp"

namespace klein::view {
    constexpr float TILE_RENDER_TICKRATE = 1. / 10.; // 10 fps

    struct RenderedTiles {
        bool active = false;
        bool dirty = true;
        sf::RenderTexture target {};
        sf::Clock render_time {};
    };

    class ViewTilesState {
    private:
        std::unordered_map<ViewKey, RenderedTiles, ViewKeyHash> offscreen_pool {};

    public:
        void render_views_offscreen(
            entt::registry& registry,
            const RaycastViewResponse& raycast,
            const sf::Vector2u resolution
        );

        void compose_views(
            sf::RenderTarget &target,
            const RaycastViewResponse& raycast
        ) const;
    };
}

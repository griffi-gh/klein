#include "klein/view/view_tiles.hpp"

#include <ranges>
#include <stdexcept>
#include <spdlog/spdlog.h>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Transform.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/StencilMode.hpp>

#include "SFML/Graphics/Color.hpp"
#include "klein/drawable.hpp"
#include "klein/tilemap/tilemap.hpp"

using klein::tilemap::TileMap;

namespace klein::view {
    void ViewTilesState::render_views_offscreen(
        entt::registry& registry,
        const RaycastViewResponse& raycast,
        const sf::Vector2u resolution
    ) {
        // TODO culling
        // TODO cache view textures

        for (auto &tex: offscreen_pool | std::views::values) {
            tex.active = false;
        }
        for (const auto &view: raycast.views | std::views::keys) {
            auto [it, inserted] = offscreen_pool.try_emplace(view);
            auto &tex = it->second;
            tex.active = true;
        }

        sf::RenderStates states;
        for (auto &[view, texture]: offscreen_pool) {
            if (!texture.active) continue;

            bool dirty = texture.render_time.getElapsedTime().asSeconds() > TILE_RENDER_TICKRATE;
            if (texture.target.getSize() != resolution) {
                if (!texture.target.resize(resolution))
                    throw new std::runtime_error("Texture::resize failed");
                dirty = true;
            }
            if (!dirty) continue;

            texture.target.clear(sf::Color { 32, 32, 32, 255 });
            for (const auto entity: registry.view<drawable_ptr, TileMap>()) {
                states.transform = sf::Transform{}.translate(view.trans.componentWiseMul(-tilemap::TILE_SCREEN_SIZE));
                render_drawable(registry, texture.target, entity, states);
            }
            texture.target.display();
        }
    }

    void ViewTilesState::compose_views(
        sf::RenderTarget &target,
        const RaycastViewResponse& raycast
    ) const {
        sf::RenderStates states {};
        states.stencilMode.stencilComparison = sf::StencilComparison::Equal;
        for (const auto &[view, meta]: raycast.views) {
            const auto &texture = offscreen_pool.at(view);
            states.stencilMode.stencilReference = sf::StencilValue((unsigned int) meta.stencil_idx);
            sf::Sprite sprite(texture.target.getTexture());
            target.draw(sprite, states);
        }
    };

}

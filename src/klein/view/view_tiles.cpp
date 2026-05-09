#include "klein/view/view_tiles.hpp"

#include <bit>
#include <ranges>
#include <stdexcept>
#include <spdlog/spdlog.h>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Transform.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/StencilMode.hpp>

#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Rect.hpp"
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/Text.hpp"
#include "SFML/Graphics/Texture.hpp"
#include "klein/drawable.hpp"
#include "klein/tilemap/tilemap.hpp"
#include "klein/view/view_raycast.hpp"
#include "klein/debug/debug.hpp"

using klein::tilemap::TileMap;


namespace klein::view {

    // TODO: im VERY much aware this is extremely over engineered
    //
    // this wouldve been better off as a basic grid-based tile renderer...
    // instead of texture-per-ViewKey (when viewkey isnt even used as part of the rendering process lmao)
    //
    // but..... whatever, it works and thats all that matters
    // (wish i had time to rewrite it from scratch though)

    void ViewTilesState::render_views_offscreen(
        entt::registry& registry,
        const RaycastViewResponse& raycast
    ) {
        if (debug::flags.tile_composer_purge) {
            offscreen_pool.clear();
        }

        for (auto &tex: offscreen_pool | std::views::values) {
            tex.active = false;
        }
        for (const auto &view: raycast.views | std::views::keys) {
            auto [it, inserted] = offscreen_pool.try_emplace(view);
            auto &tex = it->second;
            tex.active = true;
        }

        for (auto &[view, texture]: offscreen_pool) {
            if (!texture.active) continue;

            const auto &meta = raycast.views.at(view);

            // bool dirty = texture.render_time.getElapsedTime().asSeconds() > TILE_RENDER_TICKRATE;
            bool dirty = false;

            // mark as dirty when toggling debug shit
            const uint8_t current_debug_state = debug::flags.show_special ? 1 : 0;
            dirty |= texture._debug_state != current_debug_state;
            texture._debug_state = current_debug_state;

            sf::Vector2u current_resolution = texture.target.getSize();
            // minimum acceptable resolution
            const sf::Vector2u needs_resolution {
                sf::Vector2f(meta.visible_aabb_max - meta.visible_aabb_min + sf::Vector2i{1, 1})
                    .componentWiseMul(tilemap::TILE_SCREEN_SIZE)
            };
            // (round to nearest power of two on each axis)
            // TODO: cap out at window size
            const sf::Vector2u desired_resolution(
                std::min(std::bit_ceil(std::max(current_resolution.x, needs_resolution.x)), MAX_TILE_RESOLUTION),
                std::min(std::bit_ceil(std::max(current_resolution.y, needs_resolution.y)), MAX_TILE_RESOLUTION)
            );

            if ((current_resolution.x < needs_resolution.x ||
                 current_resolution.y < needs_resolution.y) &&
                 desired_resolution != current_resolution
            ) {
                if (!texture.target.resize(desired_resolution))
                    throw std::runtime_error("Texture::resize failed");

                spdlog::debug(
                    "ViewTilesState: resizing id {}; {}x{} -> {}x{}",
                    meta.stencil_idx,
                    current_resolution.x, current_resolution.y,
                    desired_resolution.x, desired_resolution.y
                );

                current_resolution = desired_resolution;
                dirty = true;
            }

            if (texture.current_min_aabb.x > meta.visible_aabb_min.x ||
                texture.current_min_aabb.y > meta.visible_aabb_min.y ||
                texture.current_max_aabb.x < meta.visible_aabb_max.x ||
                texture.current_max_aabb.y < meta.visible_aabb_max.y ||
                dirty // (if re-rendering anyway might as well update the view)
            ) {
                texture.target.setView(sf::View(sf::FloatRect {
                    sf::Vector2f(meta.visible_aabb_min).componentWiseMul(tilemap::TILE_SCREEN_SIZE),
                    sf::Vector2f(current_resolution)
                }));
                texture.current_min_aabb = meta.visible_aabb_min;
                texture.current_max_aabb = meta.visible_aabb_max;
                dirty = true;
            }

            if (!dirty) continue;

            texture.target.clear(sf::Color { 32, 32, 32, 255 });
            for (const auto entity: registry.view<drawable::drawable_ptr, TileMap>()) {
                drawable::render_drawable(registry, texture.target, entity);
            }

            texture.target.display();
            const bool _ = texture.target.generateMipmap();
        }
    }

    void ViewTilesState::compose_views(
        sf::RenderTarget &target,
        const RaycastViewResponse& raycast
    ) const {
        sf::RenderStates states {};
        states.stencilMode.stencilComparison = sf::StencilComparison::Equal;

        for (const auto &[view, meta]: raycast.views) {
            states.stencilMode.stencilReference = sf::StencilValue((unsigned int) meta.stencil_idx);

            const auto &texture = offscreen_pool.at(view);

            sf::Sprite sprite(
                texture.target.getTexture(),
                {
                    sf::Vector2i(
                        sf::Vector2f(meta.visible_aabb_min - texture.current_min_aabb)
                            .componentWiseMul(tilemap::TILE_SCREEN_SIZE)),
                    sf::Vector2i (
                        sf::Vector2f(meta.visible_aabb_max - meta.visible_aabb_min + sf::Vector2i(1, 1))
                            .componentWiseMul(tilemap::TILE_SCREEN_SIZE))
                }
            );

            sprite.setPosition(
                (sf::Vector2f(meta.visible_aabb_min) - view.trans)
                    .componentWiseMul(tilemap::TILE_SCREEN_SIZE)
            );

            if (debug::flags.debug_tile_composer) {
                sf::Color hash_color(static_cast<uint32_t>(0xA7F3C91D ^ ViewKeyHash{}(view)) | 0xff);
                sprite.setColor(hash_color);
            }

            target.draw(sprite, states);

            if (debug::flags.debug_tile_composer) {
                sf::RectangleShape r {};
                r.setFillColor(sf::Color::Transparent);
                r.setOutlineColor(sf::Color::Red);
                r.setOutlineThickness(1.);
                r.setSize({texture.target.getSize()});
                r.setPosition(sprite.getPosition());
                target.draw(r);
            }

        }
    };

}

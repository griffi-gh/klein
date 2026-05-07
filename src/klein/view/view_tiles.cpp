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
#include "klein/debug_ui.hpp"
#include "klein/drawable.hpp"
#include "klein/tilemap/tilemap.hpp"
#include "klein/view/view_raycast.hpp"

using klein::tilemap::TileMap;

namespace klein::view {
    constexpr unsigned int MAX_TILE_RESOLUTION = 4096;

    void ViewTilesState::render_views_offscreen(
        entt::registry& registry,
        const RaycastViewResponse& raycast
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

        for (auto &[view, texture]: offscreen_pool) {
            if (!texture.active) continue;

            const auto &meta = raycast.views.at(view);

            // bool dirty = texture.render_time.getElapsedTime().asSeconds() > TILE_RENDER_TICKRATE;
            bool dirty = false;

            // mark as dirty when toggling debug shit
            const uint8_t current_debug_state = debug_state.show_special ? 1 : 0;
            dirty |= texture._debug_state != current_debug_state;
            texture._debug_state = current_debug_state;

            sf::Vector2u current_resolution = texture.target.getSize();
            // minimum acceptable resolution
            const sf::Vector2u needs_resolution {
                sf::Vector2f(meta.visible_aabb_max - meta.visible_aabb_min)
                    .componentWiseMul(tilemap::TILE_SCREEN_SIZE)
            };

            if (current_resolution.x < needs_resolution.x ||
                current_resolution.y < needs_resolution.y
            ) {
                // (round to nearest power of two on each axis)
                // TODO: cap out at window size
                const sf::Vector2u desired_resolution(
                    std::min(std::bit_ceil(std::max(current_resolution.x, needs_resolution.x)), MAX_TILE_RESOLUTION),
                    std::min(std::bit_ceil(std::max(current_resolution.x, needs_resolution.y)), MAX_TILE_RESOLUTION)
                );

                if (!texture.target.resize(desired_resolution))
                    throw new std::runtime_error("Texture::resize failed");

                spdlog::debug(
                    "ViewTilesState: resizing id {}; {}x{} -> {}x{}",
                    meta.stencil_idx,
                    current_resolution.x, current_resolution.y,
                    desired_resolution.x, desired_resolution.y
                );

                current_resolution = desired_resolution;
                dirty = true;
            }

            // texture.target.setView(sf::View(sf::FloatRect {
            //     sf::Vector2f(0, 0),
            //     sf::Vector2f(current_resolution)
            // }));

            // ensure viewport match
            // TODO: more efficient viewport reuse
            // like we dont need to re-render if new viewport is smaller
            const auto current_viewport = texture.target.getView();
            const auto needs_viewport = sf::View(sf::FloatRect {
                sf::Vector2f(0, 0),
                sf::Vector2f(current_resolution)
            });
            if (current_viewport.getViewport() != needs_viewport.getViewport()) {
                texture.target.setView(needs_viewport);
                dirty = true;
            }

            if (!dirty) continue;

            texture.target.clear(sf::Color { 32, 32, 32, 255 });
            for (const auto entity: registry.view<drawable_ptr, TileMap>()) {
                // sf::RenderStates states;
                // states.transform = sf::Transform{}.translate(view.trans.componentWiseMul(-tilemap::TILE_SCREEN_SIZE));
                render_drawable(registry, texture.target, entity);
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
            sprite.setPosition(view.trans.componentWiseMul(-tilemap::TILE_SCREEN_SIZE));

            if (debug_state.debug_tile_composer) {
                sf::Color hash_color(static_cast<uint32_t>(0xA7F3C91D ^ ViewKeyHash{}(view)) | 0xff);
                sprite.setColor(hash_color);
            }

            target.draw(sprite, states);

            if (debug_state.debug_tile_composer) {
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

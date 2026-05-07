#include "klein/view/view_raycast.hpp"
#include "klein/view/raycast_impl.hpp"
#include <cstdint>
#include <optional>
#include <type_traits>
#include <utility>

#define _USE_MATH_DEFINES
#include <cmath>
#include <ranges>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Transform.hpp>
#include <SFML/System/Vector2.hpp>

#include "klein/player.hpp"
#include "klein/tilemap/tilemap.hpp"
#include "util/hash_combine.hpp"

using std::views::zip, std::views::iota;

namespace klein::view {
    ViewKey ViewKey::operator*(const ViewKey& other) const noexcept {
        return {
            .trans = trans + other.trans,
            .scale = scale.componentWiseMul(other.scale)
        };
    }

    size_t ViewKeyHash::operator()(const ViewKey& k) const noexcept {
        size_t seed = 0;
        hash_combine(seed, k.trans.x);
        hash_combine(seed, k.trans.y);
        hash_combine(seed, k.scale.x);
        hash_combine(seed, k.scale.y);
        return seed;
    }

    RaycastViewResponse raycast_view(entt::registry &registry) {
        // get player
        const auto player_view = registry.view<Player, sf::Transform>();
        auto [_, player_transform] = player_view.get(player_view.front());

        const auto player_tile = player_transform
            .transformPoint({})
            .componentWiseDiv(tilemap::TILE_SCREEN_SIZE);

        // TODO
        RaycastViewResponse response{};
        response.default_view = ViewKey{}; //todo handle this shit
        response.rays.resize(VIEW_RAY_COUNT);

        response.views.emplace(response.default_view, ViewMeta {});

        for (int i = 0; i < VIEW_RAY_COUNT; ++i){
            const float a = ((float)i / (float)VIEW_RAY_COUNT) * 2 * M_PI;

            RayPath &ray = response.rays[i];
            ray.origin_t = player_tile;
            ray.direction = { std::cos(a), std::sin(a)};

            ViewKey viewkey_accum = response.default_view;
            bool been_inside_soft_wall = false; // XXX: for correctness sake this ideally should be per-map?

            std::optional<tilemap::TilePortal> exiting_portal = std::nullopt;

            ray.hit = raycast_tiles(
                ray.origin_t,
                ray.direction,
                [&](Hit hit) mutable -> StepResult {
                    bool is_inside_soft_wall = false;

                    if (exiting_portal.has_value()) {
                        // XXX: during raycast, on each portal cross, we record the length
                        // this will be the basis for constructing the stencil buffer
                        // visibility cone would be split into "layers" wher eeach one is separated by portal crossing
                        // to achieve this wed prob have to render outermost to innermost (closest to player) poly first

                        sf::Vector2f trans(exiting_portal->trans_x, exiting_portal->trans_y);

                        auto viewkey = viewkey_accum * ViewKey(trans);
                        viewkey_accum = viewkey;

                        response.views.emplace(viewkey, ViewMeta{});
                        ray.segments.push_back(RayTransition { viewkey, hit.distance, hit.tile });

                        exiting_portal = std::nullopt;

                        return ResultContinue(trans);
                    }

                    // TODO: fix multiple maps here
                    for (auto [map_entity, map]: registry.view<tilemap::TileMap>().each()) {
                        const auto *special_layer = map.get_layer_by_name(tilemap::LAYER_SPECIAL);
                        if (!special_layer) continue;

                        const auto *tile_data = special_layer->get(hit.tile);
                        if (!tile_data) continue;
                        const auto &attributes = tile_data->attributes;

                        // walls
                        if (std::holds_alternative<tilemap::TileHard>(attributes)) {
                            return ResultBlock{};
                        } else if (std::holds_alternative<tilemap::TileSoft>(attributes)) {
                            is_inside_soft_wall = true;
                            been_inside_soft_wall = true;
                            // last_pgroup = INT_MIN;
                            continue; // actually continue just until the wall ends
                        } else if (been_inside_soft_wall && !is_inside_soft_wall) {
                            // just exited wall -> non-air
                            return ResultBlock{};
                        } else if (std::holds_alternative<tilemap::TilePortal>(attributes)) { // portals
                            const auto &portal = std::get<tilemap::TilePortal>(attributes);
                            const uint8_t ray_face_mask = 1 << std::to_underlying(hit.exit_face);
                            if (portal.face_mask & ray_face_mask) {
                                exiting_portal = std::make_optional(portal);
                                continue;
                            }
                        }
                    }

                    // just exited wall -> air
                    if (been_inside_soft_wall & !is_inside_soft_wall) return ResultBlock{};

                    return ResultContinue{};
                }
            );

            response.max_segments_depth = std::max(response.max_segments_depth, ray.segments.size());
        }

        // assign stencil idx
        for (auto [idx, view_meta]: zip(iota(0uz), response.views | std::views::values)) {
            view_meta.stencil_idx = (uint8_t)idx;
        }

        return response;
    }

    void RaycastViewResponse::draw_debug(sf::RenderTarget &target) const {
        for (const auto &ray: rays) {
            sf::Color segment_start_color = sf::Color::Red;
            segment_start_color.a = 128;
            sf::Color segment_end_color = sf::Color::Green;
            segment_end_color.a = 128;

            auto segment_start = ray.origin_t;
            for (const auto &segment : ray.segments) {
                auto segment_end = ray.origin_t + ray.direction * segment.distance;

                sf::Vertex line[] = {
                    sf::Vertex(segment_start.componentWiseMul(tilemap::TILE_SCREEN_SIZE), segment_start_color),
                    sf::Vertex(segment_end.componentWiseMul(tilemap::TILE_SCREEN_SIZE), segment_end_color)
                };
                target.draw(line, 2, sf::PrimitiveType::Lines);

                segment_start = segment_end;
            }

            float ray_length = ray.hit ? ray.hit->distance : RAYCAST_MAX_DISTANCE_TILES;
            sf::Vector2f ray_end = ray.origin_t + ray.direction * ray_length;

            sf::Vertex line[] = {
                sf::Vertex(segment_start.componentWiseMul(tilemap::TILE_SCREEN_SIZE), segment_start_color),
                sf::Vertex(ray_end.componentWiseMul(tilemap::TILE_SCREEN_SIZE), segment_end_color)
            };
            target.draw(line, 2, sf::PrimitiveType::Lines);
        }
    }
}

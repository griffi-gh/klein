#include "klein/view/view_raycast.hpp"

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
            .componentWiseDiv({32.0f, 32.0f});

        // TODO
        RaycastViewResponse response{};
        response.default_view = ViewKey{}; //todo handle this shit
        response.unique_views.insert(ViewKey{}); // insert current/default view
        response.rays.resize(VIEW_RAY_COUNT);

        for (int i = 0; i < VIEW_RAY_COUNT; ++i){
            const float a = ((float)i / (float)(VIEW_RAY_COUNT)) * 2 * M_PI;

            RayPath &ray = response.rays[i];
            ray.origin_t = player_tile;
            ray.direction = { std::cos(a), std::sin(a)};

            ViewKey last_viewkey = response.default_view;
            int last_pgroup = INT_MIN;
            bool been_inside_soft_wall = false; // XXX: for correctness sake this ideally should be per-map?

            ray.hit = raycast_tiles(
                ray.origin_t,
                ray.direction,
                [&](sf::Vector2i tile, float distance) mutable -> StepResult {
                    bool is_inside_soft_wall = false;
                    bool is_inside_portal = false;

                    // TODO: fix multiple maps here
                    for (auto [map_entity, map]: registry.view<tilemap::TileMap>().each()) {
                        const auto *special_layer = map.get_layer_by_name(tilemap::LAYER_SPECIAL);
                        if (!special_layer) continue;

                        const auto *tile_data = special_layer->get(tile);
                        if (!(tile_data && tile_data->attributes)) continue;
                        const auto &attributes = *tile_data->attributes;

                        if (attributes["type"] == "soft") {
                            is_inside_soft_wall = true;
                            been_inside_soft_wall = true;
                            last_pgroup = INT_MIN;
                            continue; // actually continue just until the wall ends
                        };
                        // just exited wall -> non-air
                        if (been_inside_soft_wall && !is_inside_soft_wall) return ResultBlock{};

                        if (attributes["type"] == "hard") {
                            return ResultBlock{};
                        } else if (attributes["type"] == "portal") {
                            is_inside_portal = true;

                            float pgroup = attributes["pgroup"];
                            if (pgroup == last_pgroup) continue;
                            last_pgroup = pgroup;

                            sf::Vector2f trans {
                                attributes["trans_x"].get<float>(),
                                attributes["trans_y"].get<float>()
                            };

                            auto viewkey = last_viewkey * ViewKey(trans);
                            last_viewkey = viewkey;

                            // XXX: during raycast, on each portal cross, we record the length
                            // this will be the basis for constructing the stencil buffer ie
                            // visibility cone would be split ito "layers" wher eeach one is separated by portal crossing
                            // to achieve this wed prob have to render outermost to innermost (closest to player) poly first
                            //
                            // unique_views is recorded to then render each view to then be drawn using the stencil buffer generated from ray transitions
                            //
                            response.unique_views.insert(viewkey);
                            ray.segments.push_back(RayTransition { viewkey, distance, tile });

                            return ResultContinue(trans);
                        }
                    }

                    // just exited wall -> air
                    if (been_inside_soft_wall & !is_inside_soft_wall) return ResultBlock{};
                    if (!is_inside_portal) last_pgroup = INT_MIN; // reset last_pgroup as soon as we leave the portal bounds into e.g. air

                    return ResultContinue{};
                }
            );

            response.max_segments_depth = std::max(response.max_segments_depth, ray.segments.size());
        }

        for (const auto &[idx, view]: zip(iota(0), response.unique_views)) {
            response.view_stencil_map[view] = idx;
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

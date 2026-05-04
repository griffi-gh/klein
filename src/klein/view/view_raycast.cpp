#include "SFML/Graphics/Color.hpp"
#include "SFML/Graphics/Transform.hpp"
#include "SFML/System/Vector2.hpp"
#include "util/hash_combine.hpp"
#include "klein/player.hpp"
#include "klein/tilemap/tilemap.hpp"
#include "klein/view/view_raycast.hpp"

namespace klein::view {
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
        auto [player_transform] = player_view.get(player_view.front());

        const auto player_tile = player_transform
            .transformPoint({})
            .componentWiseDiv({32.0f, 32.0f});

        // TODO
        RaycastViewResponse response{};
        response.unique_views.insert(ViewKey{}); // insert current/default view
        response.rays.resize(VIEW_RAY_COUNT);

        for (int i = 0; i < VIEW_RAY_COUNT; ++i){
            const float a = ((float)i / (float)(VIEW_RAY_COUNT - 1)) * 2 * 3.14;

            RayPath &ray = response.rays[i];
            ray.origin_t = player_tile;
            ray.direction = { std::cos(a), std::sin(a)};

            int last_pgroup = INT_MIN;

            bool was_inside_wall = false; // XXX: for correctness sake this ideally should be per-map?

            ray.hit = raycast_tiles(
                ray.origin_t,
                ray.direction,
                [&](sf::Vector2i tile, float distance) mutable -> StepResult {
                    // TODO: fix multiple maps here
                    for (auto [map_entity, map]: registry.view<tilemap::TileMap>().each()) {
                        const auto *special_layer = map.get_layer_by_name("_special");
                        if (!special_layer) continue;

                        const auto *tile_data = special_layer->get(tile);
                        if (!tile_data) continue;

                        // no attributes -> treat as basic wall
                        if (!tile_data->attributes) {
                            was_inside_wall = true;
                            last_pgroup = INT_MIN;
                            continue; // actually continue just until the wall ends
                        };

                        // just exited wall -> non-air
                        if (was_inside_wall) return ResultBlock{};

                        const auto &attributes = *tile_data->attributes;

                        if (attributes["type"] == "portal") {
                            float pgroup = attributes["pgroup"];
                            if (pgroup == last_pgroup) {
                                continue;
                            }
                            last_pgroup = pgroup;

                            sf::Vector2f trans {
                                attributes["trans_x"].get<float>(),
                                attributes["trans_y"].get<float>()
                            };
                            // TODO: this is incorrect for nested portals.
                            // we should combine current viewkey instead of only taking current portal's
                            ViewKey viewkey(trans);

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
                    if (was_inside_wall) return ResultBlock{};

                    last_pgroup = INT_MIN; // reset last_pgroup as soon as we leave the portal bounds into e.g. air
                    return ResultContinue{};
                }
            );

            // if (hit.has_value()) {
            //     sf::Vector2f hit_pos = ray.origin_t + ray.direction * hit->distance;
            //     sf::Vertex line[] = {
            //         sf::Vertex(ray.origin_t.componentWiseMul({32.0, 32.0}), sf::Color::White),
            //         sf::Vertex(hit_pos.componentWiseMul({32.f, 32.f}), sf::Color::White)
            //     };
            //     debug_draw_target.draw(line, 2, sf::PrimitiveType::Lines);
            // }

        }

        // spdlog::debug("unique_views cnt {}", unique_views.size());

        return response;
    }

    void raycast_view_debug(const RaycastViewResponse &response, sf::RenderTarget &target) {
        for (const auto &ray : response.rays) {

            sf::Color segment_start_color = sf::Color::Red;
            segment_start_color.a = 128;
            sf::Color segment_end_color = sf::Color::Green;
            segment_end_color.a = 128;

            auto segment_start = ray.origin_t;
            for (const auto &segment : ray.segments) {
                auto segment_end = ray.origin_t + ray.direction * segment.distance;

                sf::Vertex line[] = {
                    sf::Vertex(segment_start.componentWiseMul({32,32}), segment_start_color),
                    sf::Vertex(segment_end.componentWiseMul({32,32}), segment_end_color)
                };
                target.draw(line, 2, sf::PrimitiveType::Lines);

                segment_start = segment_end;
            }

            sf::Vector2f ray_end = ray.origin_t;
            if (ray.hit.has_value()) {
                ray_end += ray.direction * ray.hit->distance;
            } else {
                ray_end += ray.direction * RAYCAST_MAX_DISTANCE_TILES;
            }

            sf::Vertex line[] = {
                sf::Vertex(segment_start.componentWiseMul({32,32}), segment_start_color),
                sf::Vertex(ray_end.componentWiseMul({32,32}), segment_end_color)
            };
            target.draw(line, 2, sf::PrimitiveType::Lines);
        }
    }
}

#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/Transform.hpp"
#include "SFML/Graphics/Vertex.hpp"
#include "SFML/System/Vector2.hpp"
#include "klein/player.hpp"
#include "klein/raytrace_impl.hpp"
#include "klein/tilemap/tilemap.hpp"
#include "klein/raytrace_portals.hpp"
#include "spdlog/spdlog.h"
#include "util/hash_combine.hpp"

namespace klein {
    size_t ViewKeyHash::operator()(const klein::ViewKey& k) const noexcept {
        size_t seed = 0;
        hash_combine(seed, k.trans.x);
        hash_combine(seed, k.trans.y);
        hash_combine(seed, k.scale.x);
        hash_combine(seed, k.scale.y);
        return seed;
    }

    constexpr const int RAY_COUNT = 200;

    void raytrace_portals(entt::registry &registry, sf::RenderTarget &target) {
        // get player
        const auto player_view = registry.view<Player, sf::Transform>();
        auto [player_transform] = player_view.get(player_view.front());

        const auto player_tile = player_transform
            .transformPoint({})
            .componentWiseDiv({32.0f, 32.0f});

        // TODO
        std::unordered_set<ViewKey, ViewKeyHash> unique_views{};

        for (int i = 0; i < RAY_COUNT; ++i){
            const float a = ((float)i / (float)(RAY_COUNT - 1)) * 2 * 3.14;
            const sf::Vector2f direction(
                std::cos(a),
                std::sin(a)
            );

            sf::Vector2i last_portal_target(INT_MIN, INT_MIN);
            if (const auto hit = raytrace(
                player_tile,
                direction,
                [&](sf::Vector2i tile) mutable -> StepResult {
                    for (auto [map_entity, map]: registry.view<tilemap::TileMap>().each()) {
                        const auto *special_layer = map.get_layer_by_name("_special");
                        if (!special_layer) continue;

                        const auto *tile_data = special_layer->get(tile);
                        if (!tile_data) continue;

                        if (!tile_data->attributes)
                            return ResultBlock{}; // no attributes -> wall
                        const auto &attributes = *tile_data->attributes;

                        if (attributes["type"] == "portal") {
                            if (last_portal_target == tile) {
                                return ResultContinue{};
                            }

                            sf::Vector2f trans(
                                attributes["trans_x"].get<float>(),
                                attributes["trans_y"].get<float>()
                            );
                            unique_views.insert(ViewKey(trans));

                            last_portal_target = {
                                (int)std::floor(tile.x + trans.x),
                                (int)std::floor(tile.y + trans.y),
                            };
                            return ResultContinue(trans);
                        }
                    }
                    return ResultContinue{};
                }
            )) {
                sf::Vector2f hit_pos = player_tile + direction * hit->distance;
                sf::Vertex line[] = {
                    sf::Vertex(player_tile.componentWiseMul({32.0, 32.0}), sf::Color::White),
                    sf::Vertex(hit_pos.componentWiseMul({32.f, 32.f}), sf::Color::White)
                };
                target.draw(line, 2, sf::PrimitiveType::Lines);
            }

        }

        // spdlog::debug("unique_views cnt {}", unique_views.size());
    }
}

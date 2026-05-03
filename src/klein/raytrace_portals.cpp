#include "SFML/Graphics/PrimitiveType.hpp"
#include "SFML/Graphics/Transform.hpp"
#include "SFML/Graphics/Vertex.hpp"
#include "SFML/System/Vector2.hpp"
#include "klein/player.hpp"
#include "klein/raytrace_impl.hpp"
#include "klein/tilemap/tilemap.hpp"
#include <cstddef>
#include "klein/raytrace_portals.hpp"

namespace klein {
    void raytrace_portals(entt::registry &registry, sf::RenderTarget &target) {
        // get player
        const auto player_view = registry.view<Player, sf::Transform>();
        auto [player_transform] = player_view.get(player_view.front());

        const auto player_tile = player_transform
            .transformPoint({})
            .componentWiseDiv({32.0f, 32.0f});

        const int ray_count = 100;
        for (int i = 0; i < ray_count; ++i){
            const float a = ((float)i / (float)(ray_count - 1)) * 2 * 3.14;
            const sf::Vector2f direction(
                std::cos(a),
                std::sin(a)
            );
            // TODO
            if (const auto hit = raytrace(
                player_tile,
                direction,
                [&](sf::Vector2i tile){
                    for (auto [map_entity, map]: registry.view<tilemap::TileMap>().each()) {
                        const auto *special_layer = map.get_layer_by_name("_special");
                        if (!special_layer) continue;

                        const auto *tile_data = special_layer->get(tile);
                        if (!tile_data) continue;

                        if (const auto &attributes = tile_data->attributes) {
                            // TODO impl portals
                            return false;
                        } else {
                            return true;
                        }
                    }
                    return false;
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


    }
}

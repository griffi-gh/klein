#include "klein/player.hpp"

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Transform.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <entt/entt.hpp>
#include <spdlog/spdlog.h>

#include "klein/physics.hpp"
#include "klein/drawable.hpp"
#include "klein/tilemap/tilemap.hpp"
#include "klein/view/raycast.hpp"

namespace klein::player {
    entt::entity create_player_entity(entt::registry &registry, sf::Vector2f translate) {
        auto entity = registry.create();

        registry.emplace<sf::Transform>(entity, sf::Transform{}.translate(translate));

        sf::RectangleShape player_drawable({30., 60.});
        player_drawable.setOrigin(player_drawable.getSize().componentWiseMul({ 0.5, 0.5 }));
        registry.emplace<drawable::drawable_ptr>(entity,
            std::make_unique<sf::RectangleShape>(std::move(player_drawable))
        );

        registry.emplace<player::Player>(entity);
        registry.emplace<player::RespawnPoint>(entity, translate);

        registry.emplace<physics::KinematicBody>(entity, physics::KinematicBody {
            .size = player_drawable.getSize()
        });
        registry.emplace<physics::Velocity>(entity);

        return entity;
    }

    void update_player_movement(
        entt::registry& registry,
        const input::InputState &input
    ) {
        auto view = registry.view<const Player, physics::Velocity, const physics::KinematicBody>();
        for (auto [entity, player, vel, body]: view.each()) {
            vel.v.x = input.movement.x * player.move_vel;
            if (input.jump && vel.v.y == 0.0f && body.on_ground)
                vel.v.y = -player.jump_vel;
        }
    }

    void detect_player_portal_cross(entt::registry& registry, const sf::Time &dt, camera::Camera2d &camera) {
        auto view = registry.view<sf::Transform, const physics::Velocity, const Player>();
        for (auto [entity, trans, velocity, _]: view.each()) {
            const sf::Vector2f pos_cur = trans.transformPoint({});
            const sf::Vector2i tile_cur = { pos_cur.componentWiseDiv(tilemap::TILE_SCREEN_SIZE) };
            const sf::Vector2f pos_next = pos_cur + velocity.v * dt.asSeconds();
            const sf::Vector2i tile_next = { pos_next.componentWiseDiv(tilemap::TILE_SCREEN_SIZE) };

            if (tile_next == tile_cur) return;

            // determine which side it crossed
            // XXX: this is kinda hacky
            const sf::Vector2i delta = tile_next - tile_cur;
            view::TileFace crosses_side =
                (delta.x > 0) ? view::TileFace::Right :
                (delta.x < 0) ? view::TileFace::Left :
                (delta.y > 0) ? view::TileFace::Bottom :
                                view::TileFace::Top;

            for (const auto &[map_entity, map]: registry.view<const tilemap::TileMap>().each()) {
                const auto *special_layer = map.get_layer_by_name(tilemap::LAYER_SPECIAL);
                if (!special_layer) continue;

                const auto *tile_data = special_layer->get(tile_next);
                if (!tile_data) continue;

                const auto &attributes = tile_data->attributes;
                if (std::holds_alternative<tilemap::TilePortal>(attributes)) {
                    const auto &portal = std::get<tilemap::TilePortal>(attributes);
                    const uint8_t face_mask = 1 << std::to_underlying(crosses_side);
                    const sf::Vector2f port_trans(portal.trans_x, portal.trans_y);
                    const sf::Vector2f port_trans_ss = port_trans.componentWiseMul(tilemap::TILE_SCREEN_SIZE) ;
                    if (portal.face_mask & face_mask) {
                        trans = trans.translate(port_trans_ss);
                        camera.snap(port_trans_ss);
                    }
                }

            };


        }

    }
}

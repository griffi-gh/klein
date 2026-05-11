#include "klein/player.hpp"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Transform.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <entt/entt.hpp>
#include <spdlog/spdlog.h>

#include "klein/animation/animation.hpp"
#include "klein/animation/animation_loader.hpp"
#include "klein/physics.hpp"
#include "klein/drawable.hpp"
#include "klein/tilemap/tilemap.hpp"
#include "klein/view/raycast.hpp"

namespace klein::player {
    entt::entity create_player_entity(
        entt::registry &registry,
        const sf::Texture animation_texture,
        const animation::LoadedAnimations animation,
        const sf::Vector2f translate
    ) {
        auto entity = registry.create();

        registry.emplace<sf::Transform>(entity, sf::Transform{}.translate(translate));

        registry.emplace<Player>(entity);
        registry.emplace<RespawnPoint>(entity, translate);

        registry.emplace<physics::KinematicBody>(entity, PLAYER_COLLIDER_SIZE);
        registry.emplace<physics::Velocity>(entity);

        // sf::RectangleShape rect(PLAYER_COLLIDER_SIZE);
        // rect.setFillColor(sf::Color::White);
        // rect.setOrigin(rect.getSize().componentWiseMul({ 0.5, 0.5 }));
        // registry.emplace<sf::RectangleShape>(entity, rect);

        // TODO: move this to json
        animation::AnimationDrawable anim(animation_texture, animation.animations);
        anim.origin = sf::Vector2f(64., 95.);
        anim.push_animation("idle", animation::AnimationType::Sustain);
        registry.emplace<animation::AnimationDrawable>(entity, anim);

        registry.emplace<drawable::Drawable>(
            entity,
            drawable::draw_components<animation::AnimationDrawable>
        );

        return entity;
    }

    void update_player_movement(entt::registry& registry, const input::InputState &input) {
        auto view = registry.view<const Player, physics::Velocity, const physics::KinematicBody>();
        for (auto [entity, player, vel, body] : view.each()) {
            vel.v.x = input.movement.x * player.move_vel;
            if (input.jump && vel.v.y == 0.0f && body.on_ground)
                vel.v.y = -player.jump_vel;
            else if (!input.jump && vel.v.y < 0.0f)
                vel.v.y = std::max(vel.v.y, -player.min_jump_vel);
        }
    }

    void update_player_animations(
        entt::registry& registry,
        const input::InputState &input
    ) {
        auto view = registry.view<const Player, const physics::Velocity, const physics::KinematicBody, animation::AnimationDrawable>();
        for (auto [entity, player, vel, body, anim]: view.each()) {
            anim.toggle_animation("walk", body.on_ground && input.movement.x != 0);
            if (input.movement.x != 0) {
                anim.scale.x = input.movement.x < 0. ? -1.0f : 1.0f;
            }

            static bool prev_on_ground = body.on_ground;
            const bool just_landed = !prev_on_ground && body.on_ground;
            prev_on_ground = body.on_ground;

            // if in air, use air idle
            anim.toggle_animation("air", !body.on_ground, animation::AnimationType::Sustain);

            // if just jumped, push jump animation
            if (body.on_ground && input.jump && vel.v.y == 0.0f) {
                anim.push_animation("jump", animation::AnimationType::Oneshot);
            }

            // if falling, cancel jump
            // if (vel.v.y > 0.0) anim.pop_animation("jump");

            // if just landed, cancel jump and play land
            if (just_landed) {
                anim.pop_animation("jump");
                anim.push_animation("land", animation::AnimationType::Oneshot);
            }
        }
    }

    void detect_player_portal_cross(
        entt::registry& registry,
        const sf::Time &dt,
        camera::Camera2d &camera
    ) {
        auto view = registry.view<sf::Transform, const physics::Velocity, const Player>();
        for (auto [entity, trans, velocity, _]: view.each()) {
            const sf::Vector2f pos_cur = trans.transformPoint({});
            const sf::Vector2i tile_cur(pos_cur.componentWiseDiv(tilemap::TILE_SCREEN_SIZE));
            const sf::Vector2f pos_next = pos_cur + velocity.v * dt.asSeconds();
            const sf::Vector2i tile_next(pos_next.componentWiseDiv(tilemap::TILE_SCREEN_SIZE));

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

    void debug_draw_player_hitbox(
        const entt::registry& registry,
        sf::RenderTarget &target
    ) {
        for (const auto &player: registry.view<player::Player>()) {
            const auto &[body, trans] = registry.get<const physics::KinematicBody, const sf::Transform>(player);
            sf::RectangleShape hitbox_shape(body.size);
            hitbox_shape.setPosition(trans.transformPoint({}));
            hitbox_shape.setFillColor(sf::Color::Transparent);
            hitbox_shape.setOutlineColor(sf::Color::Green);
            hitbox_shape.setOutlineThickness(2.f);
            hitbox_shape.setOrigin(body.size.componentWiseMul({0.5f, 0.5f}));
            target.draw(hitbox_shape);
        }
    }
}

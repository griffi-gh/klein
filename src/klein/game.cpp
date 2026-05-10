#include "klein/game.hpp"

#include <imgui_internal.h>
#include <memory>
#include <stdexcept>
#include <imgui-SFML.h>
#include <spdlog/spdlog.h>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/StencilMode.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/WindowEnums.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Window/ContextSettings.hpp>
#include <variant>

#include "klein/drawable.hpp"
#include "klein/input.hpp"
#include "klein/physics.hpp"
#include "klein/player.hpp"
#include "klein/camera.hpp"
#include "klein/tilemap/tilemap.hpp"
#include "klein/tilemap/tilemap_drawable.hpp"
#include "klein/tilemap/tilemap_loader.hpp"
#include "klein/vfs/vfs_assets.hpp"
#include "klein/view/view_raycast.hpp"
#include "klein/view/view_tiles.hpp"
#include "klein/debug/debug.hpp"
#include "klein/debug/debug_ui.hpp"

namespace klein::game {
    void Game::run() {
        init();
        while (window.isOpen())
        {
            process_events();
            update();
            render();
        }
        shutdown();
    }

    void Game::init() {
        window = sf::RenderWindow(
            sf::VideoMode(sf::Vector2u(MAIN_CAMERA_CONFIG.base_size)),
            "klein",
            sf::State::Windowed,
            sf::ContextSettings {
                .sRgbCapable = false,
            }
        );
        window.setVerticalSyncEnabled(true);

        if (!ImGui::SFML::Init(window))
            throw std::runtime_error("ImGui init failed");

        // load spritesheet
        const auto spritesheet_path = vfs::resolve_asset_path("spritesheet.png");
        const auto spritesheet = std::make_shared<tilemap::Spritesheet>(tilemap::Spritesheet {
            .texture = sf::Texture(spritesheet_path),
            .tile_size = { 32, 32 },
        });

        //load map json
        const auto map = tilemap::load_tile_map_data("map.json.gz", "map");
        const tilemap::TileMapDrawable map_drawable(spritesheet, map);

        // Tilemap entity
        const entt::entity tilemap_entity = registry.create();
        registry.emplace<drawable::drawable_ptr>(
            tilemap_entity,
            std::make_unique<tilemap::TileMapDrawable>(std::move(map_drawable))
        );
        registry.emplace<tilemap::TileMap>(tilemap_entity, std::move(map));

        // figure out player spawn pnt
        sf::Vector2f spawn_point;
        if (const auto *layer = map.get_layer_by_name(tilemap::LAYER_SPECIAL)) {
            for (const auto &tile: layer->tiles) {
                if (!std::holds_alternative<tilemap::TilePlayerSpawn>(tile.attributes)) continue;
                spawn_point = sf::Vector2f(tile.pos).componentWiseMul(tilemap::TILE_SCREEN_SIZE);
                break;
            }
        }

        // Player entity
        const entt::entity player = player::create_player_entity(registry, spawn_point);

        // update camera to follow the player
        camera.subject = player;

        spdlog::info("init done");
    }

    void Game::process_events() {
        while (const std::optional event = window.pollEvent())
        {
            ImGui::SFML::ProcessEvent(window, *event);

            if (event->is<sf::Event::Closed>())
                window.close();

            if (const auto *resized = event->getIf<sf::Event::Resized>()) {
                sf::FloatRect area({0.f, 0.f}, sf::Vector2f(resized->size));
                window.setView(sf::View(area));
            }
        }
    }

    void Game::update() {
        static sf::Clock clock;
        const auto dt = clock.restart();

        ImGui::SFML::Update(window, dt);

#ifndef NDEBUG
        debug::debug_ui();
#endif

        input.update();

        // XXX: the exact order is quite important here
        //
        player::update_player_movement(registry, input);
        physics::update_gravity(registry, dt);
        player::detect_player_portal_cross(registry, dt, camera);
        physics::step_physics(registry, dt);

        camera.try_resize(window.getSize());
        camera.update(registry, dt);
    }

    void Game::render() {
        window.clear(sf::Color::Black);

        sf::RenderTarget& render_target = camera.render_target();
        render_target.clear(sf::Color::Transparent, {0});

        // raycast
        const auto raycast = view::raycast_view(registry);

        // draw map views
        // (todo: cache views so we can render map at lower framerate)
        view_tiles.render_views_offscreen(registry, raycast);

        // update stencil state buffer
        view_stencil.update_staging(raycast);
        if (debug::flags.enable_segments) view_stencil._debug_colorize();
        view_stencil.upload_staging();

        // draw to window stencil
        view_stencil.draw_stencil(render_target, raycast);

        // draw view textures using the stencil
        view_tiles.compose_views(render_target, raycast);

        // debug overlays
        if (debug::flags.enable_segments) view_stencil.draw_debug(render_target);
        if (debug::flags.enable_rays) raycast.draw_debug(render_target);

        // player
        drawable::render_drawable(
            registry,
            render_target,
            entt::const_runtime_view{}
                .iterate(registry.storage<player::Player>())
        );

        camera.display();
        sf::Sprite camera_sprite(camera.texture());
        window.draw(camera_sprite);

        // debug ui/imgui
        ImGui::SFML::Render(window);

        window.display();
    }

    void Game::shutdown() {
        spdlog::info("shutting down...");

        ImGui::SFML::Shutdown();
    }
}

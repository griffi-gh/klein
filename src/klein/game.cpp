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

#include "SFML/Graphics/RectangleShape.hpp"
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

        auto spritesheet_path = vfs::asset_path("spritesheet.png");

        sf::Texture texture;
        if (!texture.loadFromFile(spritesheet_path))
            throw std::runtime_error("spritesheet loading failed");

        auto spritesheet = std::make_shared<tilemap::Spritesheet>(tilemap::Spritesheet {
            .texture = texture,
            .tile_size = sf::Vector2u(32, 32),
        });
        auto map = tilemap::load_tile_map_data("map.json.gz", "map");
        tilemap::TileMapDrawable map_drawable(spritesheet, map);

        auto tilemap_entity = registry.create();
        registry.emplace<drawable::drawable_ptr>(
            tilemap_entity,
            std::make_unique<tilemap::TileMapDrawable>(std::move(map_drawable))
        );
        registry.emplace<tilemap::TileMap>(tilemap_entity, std::move(map));


        auto player_entity = registry.create();

        sf::RectangleShape player_drawable({30., 48.});
        player_drawable.setOrigin(player_drawable.getSize().componentWiseMul({ 0.5, 0.5 }));
        registry.emplace<drawable::drawable_ptr>(player_entity,
            std::make_unique<sf::RectangleShape>(std::move(player_drawable)));
        registry.emplace<player::Player>(player_entity);
        registry.emplace<sf::Transform>(player_entity,
            sf::Transform{}.translate({500., 200.}));
        registry.emplace<physics::KinematicBody>(player_entity, physics::KinematicBody {
            .size = player_drawable.getSize()
        });
        registry.emplace<physics::Velocity>(player_entity);

        camera.subject = player_entity;

        spdlog::info("init done");
    }

    void Game::process_events() {
        while (const std::optional event = window.pollEvent())
        {
            ImGui::SFML::ProcessEvent(window, *event);
            if (event->is<sf::Event::Closed>()) window.close();
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

        player::update_player_movement(registry, input);

        physics::update_gravity(registry, dt);
        physics::step_physics(registry, dt);

        camera.resize(window.getSize(), {
            .depthBits = 0,
            .stencilBits = 8,
            .sRgbCapable = window.isSrgb(),
        });
        camera.update(registry, dt);
    }

    void Game::render() {
        window.clear(sf::Color::Black, {0});

        sf::RenderTarget& render_target = camera.render_target();
        render_target.clear(sf::Color::Black, {0});

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

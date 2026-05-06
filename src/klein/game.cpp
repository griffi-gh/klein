#include "klein/game.hpp"

#include <memory>
#include <stdexcept>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/StencilMode.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/WindowEnums.hpp>
#include <imgui-SFML.h>
#include <spdlog/spdlog.h>

#include "klein/debug_ui.hpp"
#include "klein/drawable.hpp"
#include "klein/input.hpp"
#include "klein/player.hpp"
#include "klein/tilemap/tilemap.hpp"
#include "klein/tilemap/tilemap_drawable.hpp"
#include "klein/tilemap/tilemap_loader.hpp"
#include "klein/vfs/assets.hpp"
#include "klein/view/view_raycast.hpp"
#include "klein/view/view_tiles.hpp"

namespace klein {
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
            sf::VideoMode({1280, 720}),
            "klein",
            sf::State::Windowed,
            sf::ContextSettings{
                .depthBits = 0,
                .stencilBits = 8,
                .sRgbCapable = false, // todo
            }
        );
        window.setVerticalSyncEnabled(false);

        if (!ImGui::SFML::Init(window))
            throw new std::runtime_error("ImGui init failed");

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
        registry.emplace<drawable_ptr>(
            tilemap_entity,
            std::make_unique<tilemap::TileMapDrawable>(std::move(map_drawable))
        );
        registry.emplace<tilemap::TileMap>(tilemap_entity, std::move(map));

        sf::CircleShape player_drawable(10.);

        auto player_entity = registry.create();
        registry.emplace<drawable_ptr>(player_entity,
            std::make_unique<sf::CircleShape>(std::move(player_drawable)));
        registry.emplace<Player>(player_entity);
        registry.emplace<sf::Transform>(player_entity,
            sf::Transform{}.translate({500., 200.}));

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
        debug_ui();
#endif

        InputState input;
        input.update();

        // this is stub/debug code, should be moved to player controller eventually
        const float x = (input.right ? 1.0f : 0.0f) - (input.left ? 1.0f : 0.0f);
        const float y = (input.down ? 1.0f : 0.0f) - (input.up ? 1.0f : 0.0f);
        if (x != 0.0f || y != 0.0f) {
            auto view = registry.view<Player, sf::Transform>();
            for (auto entity : view) {
                auto& player = view.get<Player>(entity);
                auto& transform = view.get<sf::Transform>(entity);
                transform.translate({x * player.move_speed * dt.asSeconds(), y * player.move_speed * dt.asSeconds()});
            }
        }
    }

    void Game::render() {
        window.clear(sf::Color::Black, sf::StencilValue(0));

        // draw tilemap
        // render_drawable(
        //     registry,
        //     window,
        //     entt::const_runtime_view{}
        //         .iterate(registry.storage<tilemap::TileMap>())
        // );

        // Tilemap/world rendering

        // raycast
        const auto raycast = view::raycast_view(registry);

        // draw map views
        // (todo: cache views so we can render map at lower framerate)
        view_tiles.render_views_offscreen(registry, raycast, window.getSize());

        // update stencil state buffer
        view_stencil.update_staging(raycast);
        if (debug_state.enable_segments) view_stencil._debug_colorize();
        view_stencil.upload_staging();

        // draw to window stencil
        view_stencil.draw_stencil(window, raycast);

        // draw view textures using the stencil
        view_tiles.compose_views(window, raycast);

        // debug overlays
        if (debug_state.enable_segments) view_stencil.draw_debug(window);
        if (debug_state.enable_rays) raycast.draw_debug(window);

        // player
        render_drawable(
            registry,
            window,
            entt::const_runtime_view{}
                .iterate(registry.storage<Player>())
        );

        // debug ui/imgui
        ImGui::SFML::Render(window);

        window.display();
    }

    void Game::shutdown() {
        spdlog::info("shutting down...");
    }
}

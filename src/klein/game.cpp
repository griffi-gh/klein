#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/Rect.hpp"
#include "SFML/System/Vector2.hpp"
#include "SFML/System/Clock.hpp"
#include "klein/input.hpp"
#include "spdlog/spdlog.h"
#include <stdexcept>
#include <memory>
#include "klein/game.hpp"
#include "klein/vfs/assets.hpp"
#include "klein/tilemap/tilemap.hpp"
#include "klein/tilemap/tilemap_drawable.hpp"
#include "klein/tilemap/tilemap_loader.hpp"
#include "klein/drawable.hpp"
#include "klein/player.hpp"
#include "klein/view/view_raycast.hpp"
#include "klein/view/view_stencil.hpp"

namespace klein {
    /// Bootstraps and runs through the complete lifecycle of the game
    ///
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
        window = sf::RenderWindow(sf::VideoMode({1280, 720}), "klein");

        auto spritesheet_path = vfs::asset_path("spritesheet.png");

        sf::Texture texture;
        if (!texture.loadFromFile(spritesheet_path)) {
            throw std::runtime_error("spritesheet loading failed");
        }

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
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            if (const auto *resized = event->getIf<sf::Event::Resized>()) {
                sf::FloatRect area({0.f, 0.f}, sf::Vector2f(resized->size));
                window.setView(sf::View(area));
            }
        }
    }

    void Game::update() {
        static sf::Clock clock;
        const float dt = clock.restart().asSeconds();

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
                transform.translate({x * player.move_speed * dt, y * player.move_speed * dt});
            }
        }
    }

    void Game::render() {
        window.clear();

        render_drawable(
            registry,
            window,
            entt::const_runtime_view{}
                .iterate(registry.storage<tilemap::TileMap>())
        );
        render_drawable(
            registry,
            window,
            entt::const_runtime_view{}
                .iterate(registry.storage<Player>())
        );

        auto raycast_result = view::raycast_view(registry);

        static auto *stencil_state = new view::ViewStencilState();
        stencil_state->update_staging(raycast_result);
        stencil_state->_debug_colorize();
        stencil_state->upload_staging();
        stencil_state->_debug_draw(window);

        view::raycast_view_debug(raycast_result, window);

        window.display();
    }

    void Game::shutdown() {
        spdlog::info("shutting down...");
    }
}

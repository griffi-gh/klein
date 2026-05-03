#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/Rect.hpp"
#include "SFML/Graphics/Transform.hpp"
#include "SFML/System/Vector2.hpp"
#include "klein/player.hpp"
#include "spdlog/spdlog.h"
#include <stdexcept>
#include <memory>
#include "klein/game.hpp"
#include "klein/assets.hpp"
#include "klein/tilemap/tilemap.hpp"
#include "klein/tilemap/tilemap_drawable.hpp"
#include "klein/tilemap/tilemap_loader.hpp"
#include "klein/drawable.hpp"

struct tilemap_tag {};

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

        auto spritesheet_path = assets::resolve_path("spritesheet.png");

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
        registry.emplace<tilemap_tag>(tilemap_entity);

        sf::CircleShape player_drawable(10.);

        auto player_entity = registry.create();
        registry.emplace<drawable_ptr>(player_entity,
            std::make_unique<sf::CircleShape>(std::move(player_drawable)));
        registry.emplace<Player>(player_entity);
        registry.emplace<sf::Transform>(player_entity,
            sf::Transform{}.translate({128., 200.}));

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

    }

    void Game::render() {
        window.clear();

        render_drawable(
            registry,
            window,
            entt::const_runtime_view{}
                .iterate(registry.storage<tilemap_tag>())
        );

        render_drawable(
            registry,
            window,
            entt::const_runtime_view{}
                .iterate(registry.storage<Player>())
        );

        window.display();
    }

    void Game::shutdown() {
        spdlog::info("shutting down...");
    }
}

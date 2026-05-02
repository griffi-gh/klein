#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/Rect.hpp"
#include "SFML/System/Vector2.hpp"
#include "klein/tilemap/tilemap.hpp"
#include "klein/tilemap/tilemap_drawable.hpp"
#include "spdlog/spdlog.h"
#include "klein/game.hpp"

#include "klein/kdraw/drawable.hpp"
#include "klein/tilemap/tilemap_loader.hpp"
#include <memory>
#include <stdexcept>

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
        window = sf::RenderWindow(sf::VideoMode({1280, 720}), "klein");

        sf::Texture texture;
        if (!texture.loadFromFile("assets/spritesheet.png")) {
            throw std::runtime_error("spritesheet loading failed");
        }

        auto spritesheet = std::make_shared<tilemap::Spritesheet>(tilemap::Spritesheet {
            .texture = texture,
            .tile_size = sf::Vector2u(32, 32),
        });
        auto map = tilemap::load_tile_map_data("map.json.gz", "map");
        tilemap::TileMapDrawable map_drawable(spritesheet, map);

        auto entity = registry.create();
        registry.emplace<kdraw::Drawable>(entity, kdraw::Drawable {
            .sf_drawable = std::make_unique<tilemap::TileMapDrawable>(std::move(map_drawable))
        });
        registry.emplace<kdraw::Transform>(entity, kdraw::Transform {
            .sf_transform = sf::Transform{}
        });

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
        kdraw::render_drawables(registry, window);
        window.display();
    }

    void Game::shutdown() {
        spdlog::info("shutting down...");
    }
}

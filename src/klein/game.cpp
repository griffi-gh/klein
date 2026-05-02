#include "SFML/Graphics/CircleShape.hpp"
#include "SFML/Graphics/Rect.hpp"
#include "spdlog/spdlog.h"
#include "klein/tilemap/tilemap_loader.hpp"
#include "klein/kdraw/drawable.hpp"
#include "klein/game.hpp"

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

        sf::CircleShape shape(50.f);
        shape.setFillColor(sf::Color(100, 250, 50));

        auto entity = registry.create();
        registry.emplace<kdraw::Drawable>(entity, kdraw::Drawable {
            .sf_drawable = std::make_unique<sf::CircleShape>(std::move(shape))
        });
        registry.emplace<kdraw::Transform>(entity, kdraw::Transform {
            .sf_transform = sf::Transform{}
        });

        tilemap::load_tile_map_data("map.json.gz", "map");

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

#include "SFML/Graphics/CircleShape.hpp"
#include "entt/entt.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "app.hpp"
#include "drawie.hpp"
#include <memory>

void Game::init() {
    window = sf::RenderWindow(sf::VideoMode({1280, 720}), "thingy");

    sf::CircleShape shape(50.f);
    shape.setFillColor(sf::Color(100, 250, 50));

    auto entity = registry.create();
    registry.emplace<drawie::Drawable>(entity, drawie::Drawable {
        .sf_drawable = std::make_unique<sf::CircleShape>(std::move(shape))
    });
    registry.emplace<drawie::Transform>(entity, drawie::Transform {
        .sf_transform = sf::Transform{}
    });
}

void Game::update() {
    while (const std::optional event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>()) window.close();
    }
}

void Game::render() {
    window.clear();
    drawie::render_drawables(registry, window);
    window.display();
}

void Game::run() {
    while (window.isOpen())
    {
        update();
        render();
    }
}

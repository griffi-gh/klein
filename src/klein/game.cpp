#include <memory>
#include "SFML/Graphics/CircleShape.hpp"
#include "drawie/drawie.hpp"
#include "game.hpp"

void klein::Game::init() {
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

void klein::Game::update() {
    while (const std::optional event = window.pollEvent())
    {
        if (event->is<sf::Event::Closed>()) window.close();
    }
}

void klein::Game::render() {
    window.clear();
    drawie::render_drawables(registry, window);
    window.display();
}

void klein::Game::run() {
    while (window.isOpen())
    {
        update();
        render();
    }
}

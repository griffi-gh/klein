#pragma once
#include <entt/entity/registry.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

namespace klein {
    class Game {
    public:
        entt::registry registry = {};
        sf::RenderWindow window;
        void init();
        void update();
        void render();
        void run();
    };
}

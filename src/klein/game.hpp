#pragma once
#include <entt/entity/registry.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

namespace klein {
    class Game {
    public:
        entt::registry registry = {};
        sf::RenderWindow window;

    public:
        void run();

    private:
        void init();
        void process_events();
        void update();
        void render();
        void shutdown();
    };
}

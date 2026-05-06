#pragma once

#include <entt/entity/registry.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "klein/view/view_stencil.hpp"
#include "klein/view/view_tiles.hpp"

namespace klein {
    class Game {
    private:
        entt::registry registry = {};
        sf::RenderWindow window;

        // internal state
        view::ViewStencilState view_stencil {};
        view::ViewTilesState view_tiles {};

        void init();
        void process_events();
        void update();
        void render();
        void shutdown();

    public:
        /// Bootstraps and runs through the complete lifecycle of the game
        ///
        void run();
    };
}

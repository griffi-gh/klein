#pragma once

#include <entt/entity/registry.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "klein/camera.hpp"
#include "klein/input.hpp"
#include "klein/view/view_stencil.hpp"
#include "klein/view/view_tiles.hpp"

namespace klein::game {
    constexpr const camera::CameraConfig MAIN_CAMERA_CONFIG {
        .smooth_enable = true,
        .smooth_fac = 6.,
        .leeway_enable = true,
        .leeway = { 32., 80. },
        .base_size_option = camera::SizeOption::FitInside,
        .base_size = { 1600, 900 },
    };

    class Game {
    private:
        entt::registry registry = {};
        sf::RenderWindow window;

        // internal state
        view::ViewStencilState view_stencil {};
        view::ViewTilesState view_tiles {};
        input::InputState input {};
        camera::Camera2d camera{ MAIN_CAMERA_CONFIG };

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

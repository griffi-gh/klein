#pragma once
#include "SFML/System/Vector2.hpp"
#include <SFML/Window/Keyboard.hpp>

namespace klein::input {
    struct InputState {
        sf::Vector2f movement{};
        bool jump = false;

        void update();
    };
}

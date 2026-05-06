#pragma once
#include <SFML/Window/Keyboard.hpp>

namespace klein {
    struct InputState {
        bool up = false;
        bool down = false;
        bool left = false;
        bool right = false;

        void update();
    };
}

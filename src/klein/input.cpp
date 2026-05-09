#include "klein/input.hpp"
#include <SFML/Window/Keyboard.hpp>

namespace klein::input {
    void InputState::update() {
        const float axis_x =
            (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) ||
             sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left) ? -1.f : 0.f) +
            (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) ||
             sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right) ? 1.f : 0.f);
        const float axis_y =
            (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) ||
             sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down) ? -1.f : 0.f) +
            (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) ||
             sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) ? 1.f : 0.f);
        const bool button_a =
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) ||
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up);

        movement = sf::Vector2f(axis_x, axis_y);
        jump = button_a;
    }
}
